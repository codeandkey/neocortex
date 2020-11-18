/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "search.h"
#include "log.h"
#include "movegen.h"
#include "tt.h"
#include "eval_consts.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cassert>
#include <cmath>
#include <thread>

using namespace neocortex;

static int safe_parseint(std::vector<std::string> parts, size_t ind);

search::Search::Search(Position root) : root(root) {
	set_threads(std::thread::hardware_concurrency());
}

search::Search::~Search() {
	stop();
}

void search::Search::go(std::vector<std::string> args, std::ostream& out) {
	for (auto &i : search_threads) {
		if (i.joinable()) {
			stop();
			break;
		}
	}

	wtime = btime = winc = binc = movetime = depth = nodes = -1;
	infinite = false;

	/* Parse UCI options. */
	for (size_t i = 1; i < args.size(); ++i) {
		if (isdigit(args[i][0])) continue;

		if (args[i] == "wtime") wtime = safe_parseint(args, i + 1);
		else if (args[i] == "btime") btime = safe_parseint(args, i + 1);
		else if (args[i] == "winc") winc = safe_parseint(args, i + 1);
		else if (args[i] == "binc") binc = safe_parseint(args, i + 1);
		else if (args[i] == "depth") depth = safe_parseint(args, i + 1);
		else if (args[i] == "nodes") nodes = safe_parseint(args, i + 1);
		else if (args[i] == "movetime") movetime = safe_parseint(args, i + 1);
		else if (args[i] == "infinite") infinite = true;
		else if (args[i] == "movestogo") {} /* ignore movestogo */
		else {
			throw util::fmterr("Invalid argument: %s", args[i].c_str());
		}
	}

	neocortex_debug("Parsed all arguments.\n");

	should_stop = false;
	numnodes = 0;
	depth_starttime = util::time_now();

	search_threads.push_back(std::thread([&] { worker(out, 0, root); }));

	for (int i = 1; i < num_threads; ++i) {
		search_threads.push_back(std::thread([&] { worker(out, i, root); }));
	}
}

void search::Search::stop() {
	should_stop = true;

	for (auto &t : search_threads) {
		if (!t.joinable()) continue;

		t.join();
	}

	search_threads.clear();
}

void search::Search::load(Position p) {
	stop();
	root = p;
}

int search::Search::elapsed() {
	return util::time_elapsed_ms(search_starttime);
}

int search::Search::elapsed_iter() {
	return util::time_elapsed_ms(depth_starttime);
}

bool search::Search::is_time_expired() {
	return (allocated_time > 0) && (elapsed() >= allocated_time);
}

void search::Search::set_debug(bool enabled) {
	this->debug = enabled;
}

void search::Search::worker(std::ostream& out, int id, Position root) {
	int next_depth = 2;
	int value = 0;
	float ebf = -1.0f; /* effective branching factor, set after depth 3 */

	int ebf_nodes[PV_MAX] = { 1 };
	int ebf_times[PV_MAX] = { 1, 1 }; /* we can assume depth 0 and 1 searches take effectively 1ms */

	int ourtime = (root.get_color_to_move() == piece::WHITE) ? wtime : btime;
	int ourinc = (root.get_color_to_move() == piece::WHITE) ? winc : binc;

	Move best_move;

	if (!id) neocortex_debug("at [%s]:\n%s", root.to_fen().c_str(), Eval(root).to_table().c_str());

	/* Evaluate the position at depth 0 for an initial score. */
	if (!id) {
		out << "info depth 0 nodes 1 score " << score::to_uci(Eval(root).to_score()) << "\n";
		out.flush();
	}

	/* Search depth 1 before setting any time constraints. */
	PV first_pv;

	value = search_sync(root, 1, score::CHECKMATED, score::CHECKMATE, &first_pv);

	if (!id) {
		out << "info depth 1";

		if (first_pv.len > 1) {
			out << " seldepth " << (first_pv.len - 1);
		}
	
		out << " nodes " << numnodes << " score " << score::to_uci(value) <<  " pv " << first_pv.to_string() << "\n";
		out.flush();
	}

	ebf_nodes[1] = numnodes;

	assert(value != score::INCOMPLETE);
	best_move = first_pv.moves[0];

	/* Determine time to allocate */
	if (movetime > 0) {
		allocated_time = movetime.load();
	} else {
		if (ourtime > 0) {
			allocated_time = ourtime / search::ALLOC_FRACTION; /* TODO: actual time management */

            if (ourinc > 0) {
                allocated_time += ourinc;
            }
		}
	}
	
	search_starttime = util::time_now();
	
	while (1) {
		/* If the search should stop, break now and exit. */
		if (should_stop) break;
		if (!infinite && (depth >= 0) && next_depth > depth) break;
		if (is_time_expired()) break;

		/* If we have an EBF and can predict the time of the next iter, try and do an early exit */
		if (ebf > 0.0f) {
			if (allocated_time > 0) {
				if (util::time_elapsed_ms(search_starttime) + (int)(ebf * ebf_times[next_depth - 1]) >= allocated_time) {
					break;
				}
			}
		}

		/* OK to start next depth */
		PV next_pv;

		int next_value = search_sync(root, next_depth, score::CHECKMATED, score::CHECKMATE, &next_pv);

		if (next_value == score::INCOMPLETE) break;

		value = next_value;

		int iter_time = elapsed_iter();

		assert(iter_time >= 0);
		if (!iter_time) iter_time = 1;

		if (!id) {
			/* Write info from the results. */
			out << "info depth " << next_depth;

			if (next_pv.len > next_depth) {
				out << " seldepth " << (next_pv.len - next_depth);
			}

			out << " nodes " << numnodes;
			out << " time " << iter_time;
			out << " nps " << (int) (1000.0 * (double) numnodes / (double) iter_time);
			out << " score " << score::to_uci(value);
			out << " pv " << next_pv.to_string();
			out << "\n";
			out.flush();
		}

		ebf_nodes[next_depth] = numnodes;
		ebf_times[next_depth] = iter_time;

		if (next_depth >= 2) {
			ebf = sqrtf((float) ebf_nodes[next_depth] / (float) ebf_nodes[next_depth - 1]);
		}

		best_move = next_pv.moves[0];
		++next_depth;
	}

	/* Write the best move found. */
	assert(best_move.is_valid());

	if (!id) {
		out << util::format("bestmove %s\n", best_move.to_uci().c_str());
		out.flush();
	}
}

int search::Search::search_sync(Position& root, int depth, int alpha, int beta, PV* pv_line) {

	return alphabeta(root, depth, alpha, beta, pv_line);
}

int search::Search::alphabeta(Position& root, int depth, int alpha, int beta, PV* pv_line) {
	PV local_pv;
	Move tt_move;
	int value;

	++numnodes;
	if (nodes > 0 && numnodes > nodes) return score::INCOMPLETE;

	if (is_time_expired() || should_stop) {
		return score::INCOMPLETE;
	}

	/* Check for threefold repetition or 50-move rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	if (!depth) {
		return quiescence(root, search::QDEPTH, alpha, beta, pv_line);
	}

	int alpha_orig = alpha;
	tt::entry* entry = tt::lookup(root.get_tt_key());

	if (entry->key == root.get_tt_key() && entry->depth >= 1) {
		if (entry->depth >= depth) {
			switch (entry->type) {
			case tt::entry::EXACT:
				// Re-search under pv node to regenerate complete pv, should be fast as all should hit TT
				root.make_move(entry->pv_move);

				value = score::parent(-alphabeta(root, depth - 1, -beta, -alpha, &local_pv));

				pv_line->moves[0] = entry->pv_move;
				pv_line->len = local_pv.len + 1;

				memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
				root.unmake_move(entry->pv_move);

				return value;
			case tt::entry::LOWERBOUND:
				if (entry->value > alpha) alpha = entry->value;
				break;
			case tt::entry::UPPERBOUND:
				if (entry->value < beta) beta = entry->value;
				break;
			}
		}
		else {
			/* TT entry isn't deep enough to use, but we keep the PV move for ordering */
			if (entry->type == tt::entry::EXACT) {
				tt_move = entry->pv_move;
			}
		}
	}

	movegen::Generator g(root, tt_move);
	Move next_move;
	int num_moves = 0;

	while ((next_move = g.next()).is_valid()) {
		if (root.make_move(next_move)) {
			num_moves++;

			value = score::parent(-alphabeta(root, depth - 1, -beta, -alpha, &local_pv));

			root.unmake_move(next_move);

			if (value == score::INCOMPLETE) {
				return value;
			}

			if (value >= beta) return beta;

			if (value > alpha) {
				alpha = value;

				pv_line->moves[0] = next_move;
				pv_line->len = local_pv.len + 1;

				memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
			}
		}
		else {
			root.unmake_move(next_move);
		}
	}

	if (!num_moves) {
		if (root.check()) {
			pv_line->len = 0;
			return score::CHECKMATED;
		}
		else {
			pv_line->len = 0;
			return eval::CONTEMPT;
		}
	}

	entry->key = root.get_tt_key();
	entry->value = alpha;
	entry->depth = depth;

	if (alpha <= alpha_orig) {
		entry->type = tt::entry::UPPERBOUND;
	}
	else if (alpha >= beta) {
		entry->type = tt::entry::LOWERBOUND;
	}
	else {
		assert(pv_line->moves[0].is_valid());

		entry->pv_move = pv_line->moves[0];
		entry->type = tt::entry::EXACT;
	}

	return alpha;
}

int search::Search::quiescence(Position& root, int depth, int alpha, int beta, PV* pv_line) {
	PV local_pv;

	if (is_time_expired() || should_stop) {
		return score::INCOMPLETE;
	}

	/* Check for threefold repetition or 50-move-rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	++numnodes;
	if (nodes > 0 && numnodes > nodes) return score::INCOMPLETE;

	if (!depth || root.quiet()) {
		pv_line->len = 0;
		return Eval(root).to_score();
	}

	movegen::Generator g(root, Move::null, movegen::QUIESCENCE);
	Move next_move;
	int num_moves = 0;

	while ((next_move = g.next()).is_valid()) {
		if (root.make_move(next_move)) {
			num_moves++;

			int value = score::parent(-quiescence(root, depth - 1, -beta, -alpha, &local_pv));

			root.unmake_move(next_move);

			if (value == score::INCOMPLETE) {
				return value;
			}

			if (value >= beta) return beta;

			if (value > alpha) {
				alpha = value;

				pv_line->moves[0] = next_move;
				pv_line->len = local_pv.len + 1;

				memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
			}
		}
		else {
			root.unmake_move(next_move);
		}
	}

	if (!num_moves) {
		if (root.check()) {
			pv_line->len = 0;
			return score::CHECKMATED;
		}
		else {
			pv_line->len = 0;
			return eval::CONTEMPT;
		}
	}

	return alpha;
}

void search::Search::set_threads(int num) {
	num_threads = num;

	neocortex_debug("Using %d threads for searching.\n", num);
}

int safe_parseint(std::vector<std::string> parts, size_t ind) {
	if (ind >= parts.size()) {
		throw std::runtime_error("Expected argument!");
	}

	neocortex_debug("Parsing %s\n", parts[ind].c_str());

	return std::stoi(parts[ind]);
}
