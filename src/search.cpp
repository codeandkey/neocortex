/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "search.h"
#include "log.h"
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
	stop();

	wtime = btime = winc = binc = movetime = depth = -1;
	infinite = false;

	/* Parse UCI options. */
	for (size_t i = 1; i < args.size(); ++i) {
		if (isdigit(args[i][0])) continue;

		if (args[i] == "wtime") wtime = safe_parseint(args, i + 1);
		else if (args[i] == "btime") btime = safe_parseint(args, i + 1);
		else if (args[i] == "winc") winc = safe_parseint(args, i + 1);
		else if (args[i] == "binc") binc = safe_parseint(args, i + 1);
		else if (args[i] == "depth") depth = safe_parseint(args, i + 1);
		else if (args[i] == "movetime") movetime = safe_parseint(args, i + 1);
		else if (args[i] == "infinite") infinite = true;
		else if (args[i] == "movestogo") {} /* ignore movestogo */
		else {
			throw util::fmterr("Invalid argument: %s", args[i].c_str());
		}
	}

	neocortex_debug("Parsed all arguments.\n");

	/* Start main search thread. */
	control_should_stop = false;
	control_thread = std::thread([&] { control_worker(out, root); });
}

void search::Search::control_worker(std::ostream& out, Position root) {
	int score;
	int cur_depth;
	int ourtime, ourinc;
	int node_count, iter_time;
	PV cur_pv;
	Move best_move;

	/* EBF variables, for depth time prediction */
	float ebf = -1.0f; /* effective branching factor, set after depth 3 */
	int ebf_nodes[MAX_DEPTH] = { 1 };
	int ebf_times[MAX_DEPTH] = { 0 }; /* we can assume depth 0 and 1 searches take effectively 1ms */

	/* Determine how much time we have */
	if (root.get_color_to_move() == piece::WHITE) {
		ourtime = wtime;
		ourinc = winc;
	} else {
		ourtime = btime;
		ourinc = binc;
	}

	/* Write depth 0 information */
	out << "info depth 0 score " << score::to_uci(root.evaluate()) << "\n";
	out.flush();

	/* Search depth 1 with 1 thread */

	depth_starttime = util::time_now();

	node_count = 0;
	allocated_time = -1;

	score = alphabeta(root, 1, score::CHECKMATED, score::CHECKMATE, &cur_pv, &node_count, control_should_stop);
	iter_time = util::time_elapsed_ms(depth_starttime);

	out << "info depth 1 score " << score::to_uci(score) << " nodes " << node_count << " time " << iter_time << " pv " << cur_pv.to_string() << "\n";
	out.flush();

	/* Set search start time */
	util::time_point search_starttime = util::time_now();

	cur_depth = 2;
	while (1) {
		if (!infinite && depth > 0 && cur_depth > depth) break;
		if (!infinite && movetime > 0 && util::time_elapsed_ms(search_starttime) >= movetime) break;
		if (control_should_stop) break;
		if (cur_depth >= MAX_DEPTH) break;

		/* Allocate time for the next depth if timed */
		allocated_time = -1;

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

		/* Predict time for this depth */
		if (ebf > 0.0f) {
			if (allocated_time > 0) {
				if (util::time_elapsed_ms(search_starttime) + (int)(ebf * ebf_times[cur_depth - 1]) >= allocated_time) {
					break;
				}
			}
		}

		/* Perform search (TODO: aspiration) */

		depth_starttime = util::time_now();

		/* Start n-1 SMP threads */
		smp_should_stop = false;

		for (int i = 0; i < num_threads - 1; ++i) {
			smp_threads.push_back(std::thread([&] { smp_worker(out, cur_depth + (i % 2), root); }));
		}

		/* Search on control thread */
		node_count = 0;

		score = alphabeta(root, cur_depth, score::CHECKMATED, score::CHECKMATE, &cur_pv, &node_count, control_should_stop);

		/* Join SMP threads */
		smp_should_stop = true;

		for (auto &i : smp_threads) {
			if (i.joinable()) {
				i.join();
			}
		}

		smp_threads.clear();

		if (score == score::INCOMPLETE) break;

		/* Search complete, store best move */
		best_move = cur_pv.moves[0];
		iter_time = util::time_elapsed_ms(depth_starttime);

		/* Set EBF for next depth */
		ebf_nodes[cur_depth] = node_count;
		ebf_times[cur_depth] = iter_time;
		ebf = sqrtf((float) ebf_nodes[cur_depth] / (float) ebf_nodes[cur_depth - 1]);

		/* Write depth result to uci */
		int nps = ((unsigned long) node_count * 1000) / (iter_time + 1);

		out << "info depth " << cur_depth << " score " << score::to_uci(score) << " time " << iter_time << " nodes " << node_count << " nps " << nps << " pv " << cur_pv.to_string() << "\n";
		out.flush();

		++cur_depth;
	}

	/* Write bestmove */
	out << "bestmove " << best_move.to_uci() << "\n";
	out.flush();
}

void search::Search::stop() {
	if (control_thread.joinable()) {
		control_should_stop = true;
		control_thread.join();
		control_should_stop = false;
	}
}

void search::Search::load(Position p) {
	stop();
	root = p;
}

void search::Search::smp_worker(std::ostream& out, int s_depth, Position root) {
	PV local_pv;
	alphabeta(root, s_depth, score::CHECKMATED, score::CHECKMATE, &local_pv, NULL, smp_should_stop);
}

bool search::Search::allocated_time_expired() {
	return (allocated_time > 0 && util::time_elapsed_ms(depth_starttime) >= allocated_time);
}

int search::Search::alphabeta(Position& root, int depth, int alpha, int beta, PV* pv_line, int* node_count, std::atomic<bool>& abort_watch) {
	PV local_pv;
	Move tt_move, tt_exact_move;
	int value;

	if (allocated_time_expired() || abort_watch) {
		return score::INCOMPLETE;
	}

	/* Check for threefold repetition or 50-move rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	if (!depth) {
		return quiescence(root, search::QDEPTH, alpha, beta, pv_line, node_count);
	}

	int alpha_orig = alpha;

	/* Look up cached results in TT */

	tt::entry* entry = tt::lookup(root.get_tt_key());

	tt::lock();
	if (entry->key == root.get_tt_key() && entry->depth >= 1) {
		if (entry->depth >= depth) {
			switch (entry->type) {
			case tt::entry::EXACT:
				/* Get exact pv move from TT */
				tt_exact_move = entry->pv_move;
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
	tt::unlock();

	/* If there is an exact PV, just search under the PV move */
	if (tt_exact_move.is_valid()) {
		// Re-search under pv node to regenerate complete pv, should be fast as all should hit TT
		root.make_move(tt_exact_move);

		value = score::parent(-alphabeta(root, depth - 1, -beta, -alpha, &local_pv, node_count, abort_watch));

		pv_line->moves[0] = entry->pv_move;
		pv_line->len = local_pv.len + 1;

		memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
		root.unmake_move(tt_exact_move);

		return value;
	}

	Move pl_moves[MAX_PL_MOVES];
	int num_pl_moves = root.pseudolegal_moves(pl_moves);
	int num_moves = 0; /* num of legal moves */

	root.order_moves(pl_moves, num_pl_moves, tt_move);

	for (int i = 0; i < num_pl_moves; ++i) {
		if (root.make_move(pl_moves[i])) {
			num_moves++;

			value = score::parent(-alphabeta(root, depth - 1, -beta, -alpha, &local_pv, node_count, abort_watch));

			root.unmake_move(pl_moves[i]);

			if (value == score::INCOMPLETE) {
				return value;
			}

			if (value >= beta) return beta;

			if (value > alpha) {
				alpha = value;

				pv_line->moves[0] = pl_moves[i];
				pv_line->len = local_pv.len + 1;

				memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
			}
		}
		else {
			root.unmake_move(pl_moves[i]);
		}
	}

	if (!num_moves) {
		if (root.check(root.get_color_to_move())) {
			pv_line->len = 0;
			return score::CHECKMATED;
		}
		else {
			pv_line->len = 0;
			return eval::CONTEMPT;
		}
	}

	/* Write result to TT */
	tt::lock();
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
	tt::unlock();

	return alpha;
}

int search::Search::quiescence(Position& root, int depth, int alpha, int beta, PV* pv_line, int* node_count) {
	PV local_pv;

	/* Check for threefold repetition or 50-move-rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	if (!depth || (!root.check(root.get_color_to_move()) && !root.capture())) {
		pv_line->len = 0;
	
		if (node_count) {
			++*node_count;
		}

		return root.evaluate();
	}

	Move pl_moves[MAX_PL_MOVES];
	int num_pl_moves = root.pseudolegal_moves_quiescence(pl_moves);
	int num_moves = 0; /* num of legal moves */

	num_pl_moves = root.order_moves_quiescence(pl_moves, num_pl_moves);

	for (int i = 0; i < num_pl_moves; ++i) {
		if (root.make_move(pl_moves[i])) {
			num_moves++;

			int value = score::parent(-quiescence(root, depth - 1, -beta, -alpha, &local_pv, node_count));

			root.unmake_move(pl_moves[i]);

			if (value == score::INCOMPLETE) {
				return value;
			}

			if (value >= beta) return beta;

			if (value > alpha) {
				alpha = value;

				pv_line->moves[0] = pl_moves[i];
				pv_line->len = local_pv.len + 1;

				memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
			}
		}
		else {
			root.unmake_move(pl_moves[i]);
		}
	}

	if (!num_moves) {
		if (root.check(root.get_color_to_move())) {
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
