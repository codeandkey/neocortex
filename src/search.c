#include "search.h"
#include "tt.h"
#include "movegen.h"

static nc_eval _nc_search_q(nc_position* p, int depth, nc_eval alpha, nc_eval beta, nc_timepoint max_time);
static nc_eval _nc_search_pv(nc_position* p, int depth, nc_eval alpha, nc_eval beta, nc_movelist* pv_out, nc_timepoint max_time);

static int _nc_search_nodes;
static int _nc_search_leaves;
static nc_timepoint _nc_search_start;
static nc_timepoint _nc_search_end;
static int _nc_search_only_move;
static int _nc_search_incomplete;
static int _nc_search_abort;

nc_eval nc_search(nc_position* root, int depth, nc_movelist* pv_out, nc_timepoint max_time) {
	_nc_search_nodes = 0;
	_nc_search_leaves = 0;
	_nc_search_incomplete = 0;
	_nc_search_abort = 0;

	_nc_search_start = nc_timer_current();
	nc_eval ret = _nc_search_pv(root, depth, NC_EVAL_MIN, NC_EVAL_MAX, pv_out, max_time);
	_nc_search_end = nc_timer_current();

	return ret;
}

nc_eval _nc_search_q(nc_position* p, int depth, nc_eval alpha, nc_eval beta, nc_timepoint max_time) {
	++_nc_search_nodes;

	if (_nc_search_abort) {
		_nc_search_incomplete = 1;
		return 0;
	}

	if (nc_position_is_quiet(p) || !depth) return nc_position_get_eval(p);

	if (max_time && nc_timer_current() >= max_time) {
		_nc_search_incomplete = 1;
		return NC_EVAL_MIN;
	}

	nc_movegen gen_state;
	nc_move next_move;

	nc_movegen_start_gen(p, &gen_state);

	nc_eval best_score = NC_EVAL_MIN;
	int move_count = 0;

	while (nc_movegen_next_move(p, &gen_state, &next_move)) {
		if (!nc_position_make_move(p, next_move)) {
			nc_position_unmake_move(p, next_move);
			continue;
		}

		nc_eval score = -_nc_search_q(p, depth - 1, -beta, -alpha, max_time);
		nc_position_unmake_move(p, next_move);

		++move_count;

		if (score > best_score) best_score = score;
		if (score > alpha) alpha = score;
		if (alpha >= beta) break;
	}

	if (!move_count) {
		if (p->states[p->ply].check) return NC_EVAL_MIN;
		return NC_SEARCH_CONTEMPT;
	}

	return nc_eval_parent(best_score);
}

nc_eval _nc_search_pv(nc_position* p, int depth, nc_eval alpha, nc_eval beta, nc_movelist* pv_out, nc_timepoint max_time) {
	nc_move best_move = NC_MOVE_NULL;
	nc_eval best_score = NC_EVAL_MIN;

	++_nc_search_nodes;

	nc_movelist best_pv, current_pv;
	nc_movelist_clear(pv_out);
	nc_movelist_clear(&best_pv);

	if (_nc_search_abort) {
		_nc_search_incomplete = 1;
		return 0;
	}

	if (depth <= 0) {
		return _nc_search_q(p, NC_SEARCH_QDEPTH, alpha, beta, max_time);
	}

	nc_eval alpha_orig = alpha;

	nc_ttentry* tt = nc_tt_lookup(p->key);

#ifndef NC_NO_TT
	if (tt->key == p->key && tt->depth >= depth) {
		if (tt->type == NC_TT_EXACT) {
			nc_movelist_push(pv_out, tt->bestmove);
			return tt->score;
		}/* else if (tt->type == NC_TT_LOWERBOUND) {
			if (tt->score > alpha) alpha = tt->score;
		} else if (tt->type == NC_TT_UPPERBOUND) {
			if (tt->score < beta) beta = tt->score;
		}*/
	}
#endif

	nc_move next_move;
	nc_movegen gen_state;
	nc_movegen_start_gen(p, &gen_state);

	int move_count = 0;

	while (nc_movegen_next_move(p, &gen_state, &next_move)) {
		if (!nc_position_make_move(p, next_move)) {
			nc_position_unmake_move(p, next_move);
			continue;
		}

		/* Increment legal move counter */
		++move_count;

		int extension = depth - 1;

		/* Check extension */
		if (p->states[p->ply].check) ++extension;

		/* Perform scout search if first node */ 
		nc_eval score;

		if (nc_position_is_repetition(p)) {
			score = NC_SEARCH_CONTEMPT;
			nc_movelist_clear(&current_pv);
			nc_movelist_push(&current_pv, next_move);
		} else {
			if (move_count == 1) {
				score = -_nc_search_pv(p, extension, -beta, -alpha, &current_pv, max_time);
			} else {
				score = -_nc_search_pv(p, extension, -alpha - 1, -alpha, &current_pv, max_time);

				if (alpha < score && score < beta) {
					score = -_nc_search_pv(p, extension, -beta, -score, &current_pv, max_time);
				}
			}
		}

		nc_position_unmake_move(p, next_move);

		if (score > best_score) {
			best_move = next_move;
			best_score = score;
			memcpy(&best_pv, &current_pv, sizeof current_pv);
		}

		if (score >= alpha) {
			alpha = score;
		}

		/* Perform time control here so an aborted search will still have a PV. */
		if (max_time && nc_timer_current() >= max_time) {
			_nc_search_incomplete = 1;
			break;
		}

		if (alpha >= beta) break;
	}

	/* If there were no legal moves, return a terminal score. */
	if (!move_count) {
		if (p->states[p->ply].check) return NC_EVAL_MIN;
		return NC_SEARCH_CONTEMPT;
	}

	/* Store search result back into ttable */
	tt->score = best_score;
	tt->bestmove = best_move;
	tt->depth = depth;
	tt->key = p->key;

	if (best_score <= alpha_orig) {
		tt->type = NC_TT_UPPERBOUND;
	} else if (best_score >= beta) {
		tt->type = NC_TT_LOWERBOUND;
	} else {
		tt->type = NC_TT_EXACT;
	}

	/* Generate output pv */
	nc_movelist_push(pv_out, best_move);
	nc_movelist_concat(pv_out, &best_pv);

	/* Set only move flag */
	_nc_search_only_move = (move_count == 1);

	return nc_eval_parent(best_score);
}

int nc_search_get_nodes() {
	return _nc_search_nodes;
}

int nc_search_get_nps() {
	float seconds = nc_timer_elapsed_s(_nc_search_start);
	return (int) ((float) _nc_search_nodes / (seconds + 0.000001f));
}

int nc_search_get_time() {
	return (_nc_search_end - _nc_search_start) / (CLOCKS_PER_SEC / 1000);
}

int nc_search_was_only_move() {
	return _nc_search_only_move;
}

int nc_search_incomplete() {
	return _nc_search_incomplete;
}

void nc_search_abort() {
	_nc_search_abort = 1;
}
