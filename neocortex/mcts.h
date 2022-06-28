#pragma once

#include "search.h"

#define NC_MCTS_TREESIZE 10000000
#define NC_MCTS_TREEINCR MCTS_TREESIZE
#define NC_MCTS_INFO_INTERVAL  1000
#define NC_MCTS_TEST_INTERVAL  100
#define NC_MCTS_EXPLORATION 1.41f
#define NC_MCTS_NOISE 0.08f
#define NC_MCTS_EVAL_THRESHOLD 1200
#define NC_MCTS_EVAL_MAX       0.75

void ncMctsStop();
void ncMctsStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncMctsLoad(ncPosition* root);
