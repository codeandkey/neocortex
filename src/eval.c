#include "eval.h"
#include "piece.h"

static char _nc_eval_strbuf[32];

const char* nc_eval_tostr(nc_eval score) {
	if (score < NC_EVAL_MIN + NC_EVAL_MATE_THRESHOLD) {
		snprintf(_nc_eval_strbuf, sizeof _nc_eval_strbuf,  "mate -%d", (score - NC_EVAL_MIN + 1) / 2);
	} else if (score > NC_EVAL_MAX - NC_EVAL_MATE_THRESHOLD) {
		snprintf(_nc_eval_strbuf, sizeof _nc_eval_strbuf, "mate %d", (NC_EVAL_MAX - score + 1) / 2);
	} else {
		snprintf(_nc_eval_strbuf, sizeof _nc_eval_strbuf,  "cp %d", score);
	}

	return (const char*) _nc_eval_strbuf;
}

nc_eval nc_eval_parent(nc_eval score) {
	if (score < NC_EVAL_MIN + NC_EVAL_MATE_THRESHOLD) {
		return score + 1;
	} else if (score > NC_EVAL_MAX - NC_EVAL_MATE_THRESHOLD) {
		return score - 1;
	} else {
		return score;
	}
}

int nc_eval_is_mate(nc_eval score) {
	if (score < NC_EVAL_MIN + NC_EVAL_MATE_THRESHOLD) {
		return 1;
	} else if (score > NC_EVAL_MAX - NC_EVAL_MATE_THRESHOLD) {
		return 1;
	} else {
		return 0;
	}
}

int nc_eval_is_win(nc_eval score) {
	return nc_eval_is_mate(score) && score > 0;
}
