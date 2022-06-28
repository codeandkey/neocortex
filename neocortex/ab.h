#pragma once

#include "search.h"

#define NC_AB_INCOMPLETE -50000000
#define NC_AB_WIN 1000000
#define NC_AB_LOSS (-NC_AB_WIN)
#define NC_AB_TT_SIZE 1048576
#define NC_AB_QDEPTH 6

void ncAlphaBetaStop();
void ncAlphaBetaStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncAlphaBetaLoad(ncPosition* root);
