#pragma once

#include "zobrist.h"
#include "eval_type.h"
#include "move.h"

#define NC_TT_WIDTH 131072

#define NC_TT_EXACT	  0
#define NC_TT_LOWERBOUND 1
#define NC_TT_UPPERBOUND 2

typedef struct {
	nc_move bestmove;
	int depth;
	int type;
	nc_zkey key;
	nc_eval score;
} nc_ttentry;

nc_ttentry* nc_tt_lookup(nc_zkey key);
