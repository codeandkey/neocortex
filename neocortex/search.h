#pragma once

#include "position.h"
#include "types.h"

#define NC_SEARCH_WORKERS_MAX     16
#define NC_SEARCH_WORKERS_DEFAULT 1
#define NC_SEARCH_MCTS 0
#define NC_SEARCH_ALPHABETA 1
#define NC_SEARCH_DEFAULT NC_SEARCH_ALPHABETA
#define NC_SEARCH_PV_MAX 128

typedef struct {
    int nps;
    int nodes;
    int elapsed;
    int ctm;
    int depth;
    int score;
    int mate_score;
    ncMove pv[NC_SEARCH_PV_MAX];
} ncSearchInfo;

typedef void (*ncFnBestmove)(ncMove);
typedef void (*ncFnInfo)(ncSearchInfo);

int ncSearchType(int type);

void ncSearchStop();
void ncSearchStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncSearchLoad(ncPosition* root);
