#pragma once

#include "position.h"
#include "types.h"

#define NC_SEARCH_EVAL_THRESHOLD 1000
#define NC_SEARCH_EVAL_WEIGHT    0.85

#define NC_SEARCH_INFO_INTERVAL  1000
#define NC_EXPLORATION 1.4f

typedef struct {
    int nps;
    int nodes;
    int elapsed;
    int ctm;
    float score;
} ncSearchInfo;

typedef void (*ncFnBestmove)(ncMove);
typedef void (*ncFnInfo)(ncSearchInfo);

void ncSearchStop();
void ncSearchStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncSearchLoad(ncPosition* root);
