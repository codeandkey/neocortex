#pragma once

#include "search.h"

#define NC_AB_INCOMPLETE -50000000
#define NC_AB_WIN 1000000
#define NC_AB_LOSS (-NC_AB_WIN)
#define NC_AB_TT_SIZE 65536
#define NC_AB_PV_MAX 128

void ncAlphaBetaStop();
void ncAlphaBetaStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncAlphaBetaLoad(ncPosition* root);
