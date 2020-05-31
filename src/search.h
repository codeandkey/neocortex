#pragma once

#include "position.h"
#include "eval_type.h"
#include "move.h"
#include "timer.h"

/*
 * Position searcher
 */

#define NC_SEARCH_CONTEMPT 0
#define NC_SEARCH_QDEPTH 12

nc_eval nc_search(nc_position* root, int depth, nc_movelist* pv_line, nc_timepoint max_time);

void nc_search_abort();
int nc_search_get_nodes();
int nc_search_get_nps();
int nc_search_get_time();
int nc_search_was_only_move();
int nc_search_incomplete();
