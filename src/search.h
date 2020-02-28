#pragma once

#include "position.h"
#include "eval.h"
#include "move.h"
#include "timer.h"

/*
 * Position searcher
 */

#define NC_SEARCH_CONTEMPT -100

nc_eval nc_search(nc_position* root, int depth, nc_movelist* pv_line, nc_timepoint max_time);

int nc_search_get_nodes();
int nc_search_get_nps();
int nc_search_get_time();
