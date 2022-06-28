#pragma once

#include "position.h"
#include "types.h"

#define NC_SEARCH_WORKERS_MAX     16
#define NC_SEARCH_WORKERS_DEFAULT 6

#define NC_SEARCH_MCTS 0

#define NC_SEARCH_DEFAULT NC_SEARCH_MCTS

typedef struct {
    int nps;
    int nodes;
    int elapsed;
    int ctm;
    int depth;
    float score;
} ncSearchInfo;

typedef void (*ncFnBestmove)(ncMove);
typedef void (*ncFnInfo)(ncSearchInfo);

int ncSearchType(int type);

void ncSearchStop();
void ncSearchStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncSearchLoad(ncPosition* root);
