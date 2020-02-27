#include "eval.h"
#include "piece.h"

static char _nc_eval_strbuf[16];

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

