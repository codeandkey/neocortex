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
#include <sstream>

using namespace neocortex;

search::Search::Search(Position root) : root(root) {
	set_threads(std::thread::hardware_concurrency());
}

search::Search::~Search() {
	stop();
}

void search::Search::go(std::function<void(SearchInfo)> info, std::function<void(Move)> bestmove, int wtime, int btime, int winc, int binc, int depth, int movetime, bool infinite) {
	stop();

	this->wtime = wtime;
	this->btime = btime;
	this->winc = winc;
	this->binc = binc;
	this->depth = depth;
	this->movetime = movetime;
	this->infinite = infinite;

	/* Start main search thread. */
	control_should_stop = false;
	control_thread = std::thread([=] { control_worker(root, info, bestmove); });
}

void search::Search::control_worker(Position root, std::function<void(SearchInfo)> info, std::function<void(Move)> bestmove) {
	int score;
	int cur_depth;
	int ourtime, ourinc;
	int iter_time;
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
	SearchInfo d0;

	d0.depth = 0;
	d0.nodes = 1;
	d0.score = root.evaluate();
	d0.seldepth = -1;

	info(d0);

	/* Search depth 1 with 1 thread */
	{
		std::lock_guard<std::mutex> dst_lock(depth_starttime_mutex);
		depth_starttime = util::time_now();
	}

	allocated_time = -1;

	SearchInfo d1;

	root.reset_eval_counter();
	max_ply_searched = 0;

	score = alphabeta(root, 1, score::CHECKMATED, score::CHECKMATE, 0, &d1.pv, control_should_stop);
	
	{
		std::lock_guard<std::mutex> dst_lock(depth_starttime_mutex);
		iter_time = util::time_elapsed_ms(depth_starttime);
	}

	d1.depth = 1;
	d1.nodes = root.get_eval_counter();
	d1.seldepth = max_ply_searched;
	d1.time = iter_time;
	d1.score = score;
	best_move = d1.pv.moves[0];

	info(d1);

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

		root.reset_eval_counter();

		/* Perform search (TODO: aspiration) */
		{
			std::lock_guard<std::mutex> dst_lock(depth_starttime_mutex);
			depth_starttime = util::time_now();
		}

		max_ply_searched = 0;

		/* Start n-1 SMP threads */
		smp_should_stop = false;

		int* smp_node_counts = new int[(size_t) num_threads - 1];

		for (int i = 0; i < num_threads - 1; ++i) {
			smp_node_counts[i] = 0;
			smp_threads.push_back(std::thread([=] { smp_worker(cur_depth + (i % 2), root, smp_node_counts + i); }));
		}

		/* Search on control thread */

		SearchInfo cur;
		cur.depth = cur_depth;

		cur.score = alphabeta(root, cur_depth, score::CHECKMATED, score::CHECKMATE, 0, &cur.pv, control_should_stop);

		cur.nodes = root.get_eval_counter();

		/* Join SMP threads */
		smp_should_stop = true;

		for (auto &i : smp_threads) {
			if (i.joinable()) {
				i.join();
			}
		}

		for (int i = 0; i < num_threads - 1; ++i) {
			cur.nodes += smp_node_counts[i];
		}

		smp_threads.clear();

		if (cur.score == score::INCOMPLETE) break;

		/* Search complete, store best move */
		best_move = cur.pv.moves[0];
		cur.seldepth = max_ply_searched;

		{
			std::lock_guard<std::mutex> dst_lock(depth_starttime_mutex);
			cur.time = util::time_elapsed_ms(depth_starttime);
		}

		/* Set EBF for next depth */
		ebf_nodes[cur_depth] = cur.nodes;
		ebf_times[cur_depth] = cur.time;
		ebf = sqrtf((float) ebf_nodes[cur_depth] / (float) ebf_nodes[cur_depth - 1]);

		/* Write depth result to uci */

		info(cur);
		++cur_depth;
	}

	/* Write bestmove */
	bestmove(best_move);
}

void search::Search::stop() {
	if (control_thread.joinable()) {
		control_should_stop = true;
		control_thread.join();
		control_should_stop = false;
	}
}

void search::Search::wait() {
	control_thread.join();
}

void search::Search::load(Position p) {
	stop();
	root = p;
}

void search::Search::smp_worker(int s_depth, Position root, int* node_count_out) {
	PV local_pv;

	root.reset_eval_counter();
	alphabeta(root, s_depth, score::CHECKMATED, score::CHECKMATE, 0, &local_pv, smp_should_stop);
	
	if (node_count_out) {
		*node_count_out = root.get_eval_counter();
	}
}

bool search::Search::allocated_time_expired() {
	std::lock_guard<std::mutex> lock(depth_starttime_mutex);
	return (allocated_time > 0 && util::time_elapsed_ms(depth_starttime) >= allocated_time);
}

int search::Search::alphabeta(Position& root, int depth, int alpha, int beta, int ply_dist, PV* pv_line, std::atomic<bool>& abort_watch) {
	PV local_pv;
	Move tt_move, tt_exact_move;
	int value;
	int depth_reduction = 1;
	int depth_extension = 0;

	if (allocated_time_expired() || abort_watch) {
		return score::INCOMPLETE;
	}

	/* Check for threefold repetition or 50-move rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	if (!depth) {
		return quiescence(root, search::QDEPTH, alpha, beta, ply_dist, pv_line);
	}

	int alpha_orig = alpha;

	/* Look up cached results in TT */

	tt::entry* entry = tt::lookup(root.get_tt_key());

	tt::lock(root.get_tt_key());
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
	tt::unlock(root.get_tt_key());

	/* If there is an exact PV, just search under the PV move */
	if (tt_exact_move.is_valid()) {
		// Re-search under pv node to regenerate complete pv, should be fast as all should hit TT
		root.make_move(tt_exact_move);

		value = -alphabeta(root, depth + depth_extension - depth_reduction, -beta, -alpha, ply_dist + 1, &local_pv, abort_watch);

		pv_line->moves[0] = tt_exact_move;
		pv_line->len = local_pv.len + 1;

		memcpy(pv_line->moves + 1, local_pv.moves, local_pv.len * sizeof local_pv.moves[0]);
		root.unmake_move(tt_exact_move);

		return value;
	}

	/* Perform null move reduction if available */
	if (root.null_move_allowed() && depth >= search::NULL_MOVE_REDUCTION) {
		root.make_move(Move::null);
		int nm_score = -alphabeta(root, depth - search::NULL_MOVE_REDUCTION, -beta, -alpha, ply_dist + 1, &local_pv, abort_watch);
		root.unmake_move(Move::null);

		if (nm_score >= beta) {
			return beta; // cutoff
		}
	}

	/* Check / capture extension */
	if (depth >= 5) { // don't do excessive extensions near horizon
		if (root.check() || root.capture()) {
			depth_extension++;
		}
	}

	Move pl_moves[MAX_PL_MOVES];
	int num_pl_moves = root.pseudolegal_moves(pl_moves);
	int num_moves = 0; /* num of legal moves */

	root.order_moves(pl_moves, num_pl_moves, tt_move);

	for (int i = 0; i < num_pl_moves; ++i) {
		if (root.make_move(pl_moves[i])) {

			num_moves++;

			int new_depth = depth + depth_extension - depth_reduction;

			if (new_depth < 0) {
				new_depth = 0;
			}

			value = -alphabeta(root, new_depth, -beta, -alpha, ply_dist + 1, &local_pv, abort_watch);

			/*if (depth == 1) {
				neocortex_debug("Made move %s, resulting score %d\n", pl_moves[i].to_uci().c_str(), value);
			}*/

			root.unmake_move(pl_moves[i]);

			if (value == score::INCOMPLETE) {
				return value;
			}

			if (value >= beta) {
				// Add history bonus for cutoff in normal AB
				root.add_history_bonus(pl_moves[i], depth);
				return beta;
			}

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
		if (root.check()) {
			pv_line->len = 0;
			return score::CHECKMATED + ply_dist;
		}
		else {
			pv_line->len = 0;
			return eval::CONTEMPT;
		}
	}

	/* Write result to TT */
	tt::lock(root.get_tt_key());
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
	tt::unlock(root.get_tt_key());

	return alpha;
}

int search::Search::quiescence(Position& root, int depth, int alpha, int beta, int ply_dist, PV* pv_line) {
	PV local_pv;

	/* Check for threefold repetition or 50-move-rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	int cur_score = root.evaluate();

	if (!depth) {
		return quiescence_captures(root, alpha, beta, ply_dist, pv_line);
	}

	/* Perform standing pat */

	if (!root.check()) {
		if (cur_score >= beta) {
			pv_line->len = 0;

			//neocortex_debug("Returning standing pat score %d\n", cur_score);
			return beta;
		}

		if (alpha < cur_score) {
			//neocortex_debug("Setting alpha to standing pat %d\n", cur_score);
			alpha = cur_score;
		}
	}

	Move pl_moves[MAX_PL_MOVES];
	int num_pl_moves = root.pseudolegal_moves_quiescence(pl_moves);
	int num_moves = 0; /* num of legal moves */

	num_pl_moves = root.order_moves_quiescence(pl_moves, num_pl_moves);

	for (int i = 0; i < num_pl_moves; ++i) {
		if (root.make_move(pl_moves[i])) {
			num_moves++;

			int value = -quiescence(root, depth - 1, -beta, -alpha, ply_dist + 1, &local_pv);

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
		if (root.check()) {
			pv_line->len = 0;
			return score::CHECKMATED + ply_dist;
		}
		else {
			/* In qsearch if there are no Q moves, we can't assume the game is a draw, but we cannot search further.
			 * Here we just return the static eval of the node. */
			pv_line->len = 0;

			return cur_score;
		}
	}

	return alpha;
}

int search::Search::quiescence_captures(Position& root, int alpha, int beta, int ply_dist, PV* pv_line) {
	PV local_pv;

	/* Check for threefold repetition or 50-move-rule */
	if (root.num_repetitions() >= 3 || root.halfmove_clock() >= 50) {
		return eval::CONTEMPT;
	}

	int cur_score = root.evaluate();

	/* Perform standing pat */

	if (!root.check()) {
		if (cur_score >= beta) {
			pv_line->len = 0;

			/* Set longest line */
			if (ply_dist > max_ply_searched) {
				max_ply_searched = ply_dist;
			}

			//neocortex_debug("Returning standing pat score %d\n", cur_score);
			return beta;
		}

		if (alpha < cur_score) {
			//neocortex_debug("Setting alpha to standing pat %d\n", cur_score);
			alpha = cur_score;
		}
	}

	Move pl_moves[MAX_PL_MOVES];
	int num_pl_moves = root.pseudolegal_moves_quiescence_captures(pl_moves);
	int num_moves = 0; /* num of legal moves */

	num_pl_moves = root.order_moves_quiescence(pl_moves, num_pl_moves);

	for (int i = 0; i < num_pl_moves; ++i) {
		if (root.make_move(pl_moves[i])) {
			num_moves++;

			int value = -quiescence_captures(root, -beta, -alpha, ply_dist + 1, &local_pv);

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
		if (root.check()) {
			pv_line->len = 0;
			return score::CHECKMATED + ply_dist;
		}
		else {
			/* In qsearch if there are no Q moves, we can't assume the game is a draw, but we cannot search further.
			 * Here we just return the static eval of the node. */
			pv_line->len = 0;

			/* Set longest line */
			if (ply_dist > max_ply_searched) {
				max_ply_searched = ply_dist;
			}

			return cur_score;
		}
	}

	return alpha;
}

void search::Search::set_threads(int num) {
	if (num < 1) num = 1;
	if (num > max_threads()) num = max_threads();

	num_threads = num;

	neocortex_info("Using %d search threads.\n", num);
}

int search::Search::max_threads() {
	return std::thread::hardware_concurrency();
}

bool search::Search::is_running() {
	return control_thread.joinable();
}

Position search::Search::get_position() {
	return root;
}
