#include "search.h"
#include "log.h"
#include "movegen.h"
#include "tt.h"
#include "eval_consts.h"

#include <algorithm>
#include <cstring>
#include <cassert>
#include <cmath>

using namespace pine;

search::Search::Search(Position root) : root(root)
{
	clear_go_params();
	stop_requested = false;
}

search::Search::~Search() {
	stop();
}

void search::Search::set_wtime(int wtime)
{
	this->wtime = wtime;
}

void search::Search::set_btime(int btime)
{
	this->btime = btime;
}

void search::Search::set_winc(int winc)
{
	this->winc = winc;
}

void search::Search::set_binc(int binc)
{
	this->binc = binc;
}

void search::Search::set_depth(int depth)
{
	this->depth = depth;
}

void search::Search::set_nodes(int nodes)
{
	this->nodes = nodes;
}

void search::Search::set_movetime(int movetime)
{
	this->movetime = movetime;
}

void search::Search::set_debug(bool debug)
{
	this->debug = debug;
}

void search::Search::clear_go_params() {
	wtime = btime = winc = binc = movetime = depth = nodes = movetime = allocated_time = -1;
	infinite = stop_requested = false;
}

void search::Search::go(std::ostream& uci_out) {
	if (search_thread.joinable()) {
		stop();
		search_thread.join();
	}

	stop_requested = false;
	search_thread = std::thread([&] { worker(uci_out); });
}

void search::Search::stop() {
	if (!search_thread.joinable()) {
		return;
	}

	stop_requested = true;
	search_thread.join();
}

void search::Search::load(Position p) {
	stop();
	root = p;
}

int search::Search::elapsed() {
	return util::elapsed_ms(search_starttime);
}

int search::Search::elapsed_iter() {
	return util::elapsed_ms(iter_starttime);
}

bool search::Search::is_time_expired() {
	return (allocated_time > 0) && (elapsed() >= allocated_time);
}

void search::Search::worker(std::ostream& out) {
	int next_depth = 2;
	int value = 0;
	float ebf = -1.0f; /* effective branching factor, set after depth 3 */

	int ebf_nodes[PV_MAX] = { 1 };
	int ebf_times[PV_MAX] = { 1, 1 }; /* we can assume depth 0 and 1 searches take effectively 1ms */

	int ourtime = (root.get_color_to_move() == piece::WHITE) ? wtime : btime;
	int ourinc = (root.get_color_to_move() == piece::WHITE) ? winc : binc;

	Move best_move;

	pine_info("snapshot [%s]:\n%s", root.to_fen().c_str(), Eval(root).to_table().c_str());

	/* Evaluate the position at depth 0 for an initial score. */
	out << "info depth 0 nodes 1 score " << score::to_uci(Eval(root).to_score()) << "\n";
	out.flush();

	/* Search depth 1 before setting any time constraints. */
	PV first_pv;

	value = search_sync(1, score::CHECKMATED, score::CHECKMATE, &first_pv);

	out << "info depth 1";
	
	if (first_pv.len > 1) {
		out << " seldepth " << (first_pv.len - 1);
	}
	
	out << " nodes " << ctr_nodes << " score " << score::to_uci(value) <<  " pv " << first_pv.to_string() << "\n";
	out.flush();

	ebf_nodes[1] = ctr_nodes;

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
	
	search_starttime = util::now();
	
	while (1) {
		/* If the search should stop, break now and exit. */
		if (stop_requested) break;
		if (!infinite && (depth >= 0) && next_depth > depth) break;
		if (is_time_expired()) break;

		/* If we have an EBF and can predict the time of the next iter, try and do an early exit */
		if (ebf > 0.0f) {
			if (allocated_time > 0) {
				if (util::elapsed_ms(search_starttime) + (int)(ebf * ebf_times[next_depth - 1]) >= allocated_time) {
					break;
				}
			}
		}

		/* OK to start next depth */
		PV next_pv;

		int next_value = search_sync(next_depth, score::CHECKMATED, score::CHECKMATE, &next_pv);

		if (next_value == score::INCOMPLETE) break;

		value = next_value;

		/* Write info from the results. */
		out << "info depth " << next_depth;

		if (next_pv.len > next_depth) {
			out << " seldepth " << (next_pv.len - next_depth);
		}

		out << " nodes " << ctr_nodes;
		out << " time " << elapsed_iter();
		out << " nps " << (ctr_nodes * 1000) / (elapsed_iter() + 1);
		out << " score " << score::to_uci(value);
		out << " pv " << next_pv.to_string();
		out << "\n";
		out.flush();

		ebf_nodes[next_depth] = ctr_nodes;
		ebf_times[next_depth] = (elapsed_iter());

		if (next_depth >= 2) {
			ebf = sqrtf((float) ebf_nodes[next_depth] / (float) ebf_nodes[next_depth - 1]);
		}

		best_move = next_pv.moves[0];
		++next_depth;
	}

	/* Write the best move found. */
	assert(best_move.is_valid());

	out << util::format("bestmove %s\n", best_move.to_uci().c_str());
	out.flush();
}

int search::Search::search_sync(int depth, int alpha, int beta, PV* pv_line) {
	ctr_nodes = 0;
	iter_starttime = util::now();

	return alphabeta(depth, alpha, beta, pv_line);
}

int search::Search::alphabeta(int depth, int alpha, int beta, PV* pv_line) {
	PV local_pv;
	Move tt_move;
	int value;

	++ctr_nodes;
	if (nodes > 0 && ctr_nodes > (unsigned) nodes) return score::INCOMPLETE;

	if (is_time_expired() || stop_requested) {
		return score::INCOMPLETE;
	}

	/* Check for threefold repetition */
	if (root.num_repetitions() >= 3) {
		return eval::CONTEMPT;
	}

	if (!depth) {
		return quiescence(search::QDEPTH, alpha, beta, pv_line);
	}

	int alpha_orig = alpha;
	tt::entry* entry = tt::lookup(root.get_tt_key());

	if (entry->key == root.get_tt_key() && entry->depth >= 1) {
		if (entry->depth >= depth) {
			switch (entry->type) {
			case tt::entry::EXACT:
				// Re-search under pv node to regenerate complete pv, should be fast as all should hit TT
				root.make_move(entry->pv_move);

				value = score::parent(-alphabeta(depth - 1, -beta, -alpha, &local_pv));

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

			value = score::parent(-alphabeta(depth - 1, -beta, -alpha, &local_pv));

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

int search::Search::quiescence(int depth, int alpha, int beta, PV* pv_line) {
	PV local_pv;

	if (is_time_expired() || stop_requested) {
		return score::INCOMPLETE;
	}

	/* Check for threefold repetition */
	if (root.num_repetitions() >= 3) {
		return eval::CONTEMPT;
	}

	++ctr_nodes;
	if (nodes > 0 && ctr_nodes > (unsigned) nodes) return score::INCOMPLETE;

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

			int value = score::parent(-quiescence(depth - 1, -beta, -alpha, &local_pv));

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
