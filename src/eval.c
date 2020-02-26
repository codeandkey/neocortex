#include "eval.h"
#include "piece.h"

static char _nc_eval_strbuf[16];

static nc_eval _nc_eval_material_values[6] = {
    100,
    500,
    300,
    340,
    740,
    0,
};

nc_eval nc_eval_material(nc_position* p, nc_color c) {
    nc_eval sum = 0;

    for (int t = 0; t < 5; ++t) {
        sum += __builtin_popcountll(p->piece[t] & p->color[c]) * _nc_eval_material_values[t];
    }

    return sum;
}

nc_eval nc_eval_position(nc_position* p) {
    /* TODO: better evaluation function */
    return nc_eval_material(p, p->color_to_move) - nc_eval_material(p, nc_colorflip(p->color_to_move));
}

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
