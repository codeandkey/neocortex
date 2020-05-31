#pragma once

/*
 * Evaluation heuristic
 */

#include <limits.h>

#define NC_EVAL_MATE_THRESHOLD 512
#define NC_EVAL_MAX 2000000
#define NC_EVAL_MIN (-NC_EVAL_MAX)

typedef int nc_eval;

const char* nc_eval_tostr(nc_eval score);
nc_eval nc_eval_parent(nc_eval score);
int nc_eval_is_mate(nc_eval score);
int nc_eval_is_win(nc_eval score);
