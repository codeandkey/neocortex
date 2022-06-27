#pragma once

#include "position.h"
#include "types.h"

#define NC_SEARCH_EVAL_THRESHOLD 1200
#define NC_SEARCH_EVAL_MAX       0.75

#define NC_SEARCH_WORKERS_MAX     16
#define NC_SEARCH_WORKERS_DEFAULT 8

#define NC_SEARCH_TREESIZE 10000000
#define NC_SEARCH_TREEINCR NC_SEARCH_TREESIZE

#define NC_SEARCH_INFO_INTERVAL  1000
#define NC_SEARCH_TEST_INTERVAL  100
#define NC_EXPLORATION 1.41f

#define NC_SEARCH_NOISE 0.08f

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

void ncSearchStop();
void ncSearchStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info);
void ncSearchLoad(ncPosition* root);
