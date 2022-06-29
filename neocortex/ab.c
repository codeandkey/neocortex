#include "ab.h"

#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int depth;
    int value;
    ncMove bestmove;
    ncHashKey key;
    pthread_mutex_t lock;
} ncTTNode;

typedef struct {
    pthread_t handle;
    pthread_mutex_t lock;
    int running;
    int stop;
    ncPosition position;
} ncAlphaBetaWorker;

typedef struct {
    pthread_mutex_t* lock;
    int* dst;
} ncIntSync;

static inline int ncIntSyncRead(ncIntSync* t)
{
    pthread_mutex_lock(t->lock);
    int value = *(t->dst);
    pthread_mutex_unlock(t->lock);
    return value;
}

int ncIntSyncWrite(ncIntSync* t, int value)
{
    pthread_mutex_lock(t->lock);
    int rv = *(t->dst);
    *(t->dst) = value;
    pthread_mutex_unlock(t->lock);
    return rv;
}

static int search_running;
static int search_iterations;
static int search_movetime;
static int search_stop;
static int search_tt_init;
static ncPosition search_position;
static ncFnBestmove search_cb_bestmove;
static ncFnInfo search_fn_info;
static pthread_mutex_t search_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t search_thread;
static ncAlphaBetaWorker search_workers[NC_SEARCH_WORKERS_MAX];
static int num_search_workers = NC_SEARCH_WORKERS_DEFAULT;
static ncTTNode search_tt[NC_AB_TT_SIZE];

void* ncAlphaBetaMain(void*);
void* ncAlphaBetaWorkerMain(void*);
int ncAlphaBeta(ncPosition* pos, int depth, int alpha, int beta, ncIntSync* stop, ncMove* pv, int* nodes);
int ncAlphaBetaQ(ncPosition* pos, int depth, int alpha, int beta, ncIntSync* stop, int* nodes);

static inline long milliseconds()
{
    struct timespec tm;
    timespec_get(&tm, TIME_UTC);
    return 1000 * (long) tm.tv_sec + (long) tm.tv_nsec / 1000000;
}

void ncAlphaBetaStart(int nodes, int movetime, ncFnBestmove bm, ncFnInfo info)
{
    if (!search_tt_init)
    {
        memset(search_tt, 0, sizeof search_tt);

        for (int i = 0; i < NC_AB_TT_SIZE; ++i)
            pthread_mutex_init(&search_tt[i].lock, NULL);

        search_tt_init = 1;
    }

    pthread_mutex_lock(&search_mutex);
    if (search_running)
    {
        fprintf(stderr, "error: ncSearchMcts() called with search running\n");
        pthread_mutex_unlock(&search_mutex);
        return;
    }

    search_cb_bestmove = bm;
    search_fn_info = info;
    search_iterations = nodes;
    search_movetime = movetime;
    search_stop = 0;

    pthread_mutex_unlock(&search_mutex);
    pthread_create(&search_thread, NULL, ncAlphaBetaMain, NULL);
}

void ncAlphaBetaStop()
{
    pthread_mutex_lock(&search_mutex);

    if (!search_running)
    {
        pthread_mutex_unlock(&search_mutex);
        return;
    }

    search_stop = 1;
    pthread_mutex_unlock(&search_mutex);

    // Wait for search to terminate
    while (1)
    {
        pthread_mutex_lock(&search_mutex);

        if (!search_running)
        {
            search_stop = 0;
            pthread_mutex_unlock(&search_mutex);
            return;
        }

        pthread_mutex_unlock(&search_mutex);
    }
}

void* ncAlphaBetaWorkerMain(void* w)
{
    ncAlphaBetaWorker* self = (ncAlphaBetaWorker*) w;

    pthread_mutex_lock(&self->lock);
    self->stop = 0;
    self->running = 1;
    pthread_mutex_unlock(&self->lock);

    ncIntSync stop;
    stop.lock = &self->lock;
    stop.dst = &self->stop;

    for (int d = 0;; ++d)
    {
        pthread_mutex_lock(&self->lock);
        if (self->stop)
        {
            self->running = 0;
            pthread_mutex_unlock(&self->lock);
            break;
        }
        pthread_mutex_unlock(&self->lock);
        ncMove pv[d];
        int nodes;
        ncAlphaBeta(&self->position, d, NC_AB_LOSS, NC_AB_WIN, &stop, pv, &nodes);
    }

    return NULL;
}

void* ncAlphaBetaTimeWatch(void* pstop)
{
    ncIntSync* stop = (ncIntSync*) pstop;

    pthread_mutex_lock(&search_mutex);
    int mtime = search_movetime;
    pthread_mutex_unlock(&search_mutex);

    if (mtime <= 0)
        return NULL;

    long starttime = milliseconds();

    struct timespec tm;
    tm.tv_sec = 0;
    tm.tv_nsec = (long) 1000000 * 100;

    while (milliseconds() - starttime < mtime && !ncIntSyncRead(stop))
        nanosleep(&tm, NULL);

    ncIntSyncWrite(stop, 1);
    return NULL;
}

void* ncAlphaBetaMain(void* unused)
{
    pthread_mutex_lock(&search_mutex);
    ncPosition pos = search_position;
    search_stop = 0;
    search_running = 1;

    int movetime = search_movetime;
    int maxnodes = search_iterations;

    pthread_mutex_unlock(&search_mutex);

    char ff[100];
    ncPositionToFen(&pos, ff, sizeof(ff));

    printf("info string Searching %s\n", ff);

    long starttime = milliseconds();
    long infotime = starttime;

    int lastnodes = 0;

    // Spin up workers
    for (int i = 0; i < num_search_workers - 1; ++i)
    {
        search_workers[i].position = pos;
        pthread_mutex_init(&search_workers[i].lock, NULL);
        pthread_create(&search_workers[i].handle, NULL, ncAlphaBetaWorkerMain, &search_workers[i]);
    }

    int total_nodes = 0;
    ncMove best_move = NC_NULL;

    ncIntSync search_stop_sync;
    search_stop_sync.lock = &search_mutex;
    search_stop_sync.dst = &search_stop;

    pthread_t search_timer_thread;
    pthread_create(&search_timer_thread, NULL, ncAlphaBetaTimeWatch, (void*) &search_stop_sync);

    for (int d = 0;; ++d)
    {
        if (search_iterations > 0 && total_nodes >= maxnodes)
            break;

        if (movetime >= 0 && (milliseconds() - starttime >= movetime))
            break;

        pthread_mutex_lock(&search_mutex);
        if (search_stop)
        {
            pthread_mutex_unlock(&search_mutex);
            break;
        }
        pthread_mutex_unlock(&search_mutex);

        ncSearchInfo inf;
        memset(&inf, 0, sizeof(inf));

        int value = ncAlphaBeta(&pos, d, NC_AB_LOSS, NC_AB_WIN, &search_stop_sync, inf.pv, &total_nodes);

        if (value == NC_AB_INCOMPLETE)
            break;

        inf.nodes = total_nodes;
        inf.elapsed = milliseconds() - starttime;
        inf.nps = (1000 * (long) (inf.nodes - lastnodes)) / (long) (1 + (milliseconds() - infotime));
        inf.ctm = ncPositionGetCTM(&pos);
        inf.depth = d;
        inf.score = value;
        inf.mate_score = 0;

        if (inf.ctm == NC_BLACK)
            inf.score = -inf.score;

        lastnodes = inf.nodes;
        infotime = milliseconds();

        if (search_fn_info)
            search_fn_info(inf);

        best_move = inf.pv[0];
    }
    
    // Stop workers
    ncIntSyncWrite(&search_stop_sync, 1);
    pthread_join(search_timer_thread, NULL);

    for (int i = 0; i < num_search_workers - 1; ++i)
    {
        pthread_mutex_lock(&search_workers[i].lock);
        search_workers[i].stop = 1;
        pthread_mutex_unlock(&search_workers[i].lock);
    }

    for (int i = 0; i < num_search_workers - 1; ++i)
    {
        pthread_join(search_workers[i].handle, NULL);
        pthread_mutex_destroy(&search_workers[i].lock);
    }

    pthread_mutex_lock(&search_mutex);
    search_running = 0;
    pthread_mutex_unlock(&search_mutex);

    assert(ncMoveValid(best_move));

    if (search_cb_bestmove)
        search_cb_bestmove(best_move);

    return NULL;
}

int ncAlphaBeta(ncPosition* pos, int depth, int alpha, int beta, ncIntSync* stop, ncMove* pv, int* nodes)
{
    // Check extension
    if (ncPositionIsCheck(pos))
        ++depth;

    ++*nodes;

    for (int i = 0; i < depth; ++i)
        pv[i] = NC_NULL;

    ncMove pv_move = NC_NULL;

    ncHashKey key = ncPositionGetKey(pos);
    ncTTNode* tt = &search_tt[key % NC_AB_TT_SIZE];
    pthread_mutex_lock(&tt->lock);

    if (tt->key == key && tt->depth >= depth)
    {
        // Set PV move
        pv_move = tt->bestmove;

        if (tt->depth >= depth)
        {
            int val = tt->value;
            pthread_mutex_unlock(&tt->lock);

            if (depth)
                pv[0] = pv_move;

            return val;
        }
    }
    pthread_mutex_unlock(&tt->lock);

    if (!depth)
    {
        if (ncPositionIsCheck(pos) || ncPositionIsCapture(pos))
            return ncAlphaBetaQ(pos, NC_AB_QDEPTH, alpha, beta, stop, nodes);

        int score = ncPositionEvaluate(pos);

        if (ncPositionGetCTM(pos) == NC_BLACK)
            return -score;

        return score;
    }

    ncMove moves[NC_MAX_PL_MOVES];
    int nmoves = ncPositionPLMoves(pos, moves);
    ncPositionOrderMoves(pos, moves, nmoves);

    ncMove local_pv[depth - 1];

    // Bring PV move to front if possible
    if (ncMoveValid(pv_move))
    {
        for (int i = 1; i < nmoves; ++i)
        if (moves[i] == pv_move)
        {
            moves[i] = moves[0];
            moves[0] = pv_move;
            break;
        }
    }

    for (int i = 0; i < nmoves; ++i)
    {
        if (!ncPositionMakeMove(pos, moves[i]))
        {
            ncPositionUnmakeMove(pos);
            continue;
        }

        if (!ncMoveValid(pv[0]))
            pv[0] = moves[i];

        int score = ncAlphaBeta(pos, depth - 1, -beta, -alpha, stop, local_pv, nodes);

        ncPositionUnmakeMove(pos);

        if (score == NC_AB_INCOMPLETE)
            return NC_AB_INCOMPLETE;

        if (ncIntSyncRead(stop))
            return NC_AB_INCOMPLETE;

        score = -score;

        if (score > alpha)
        {
            alpha = score;
            pv[0] = moves[i];
            memcpy(&pv[1], local_pv, sizeof(local_pv));
        }

        if (score >= beta)
            return score;
    }

    int value = alpha;

    if (!ncMoveValid(pv[0]))
    {
        if (ncPositionIsCheck(pos))
            value = NC_AB_LOSS;
        else
            value = 0;
    }

    // Update TT entry
    pthread_mutex_lock(&tt->lock);
    tt->key = key;
    tt->bestmove = pv[0];
    tt->value = value;
    tt->depth = depth;
    pthread_mutex_unlock(&tt->lock);

    return value;
}

int ncAlphaBetaQ(ncPosition* pos, int depth, int alpha, int beta, ncIntSync* stop, int* nodes)
{
    ++*nodes;

    if (!depth || (!ncPositionIsCheck(pos) && !ncPositionIsCapture(pos)))
    {
        int score = ncPositionEvaluate(pos);

        if (ncPositionGetCTM(pos) == NC_BLACK)
            return -score;

        return score;
    }

    ncMove moves[NC_MAX_PL_MOVES];
    int nmoves = ncPositionPLMovesQ(pos, moves);
    //ncPositionOrderMoves(pos, moves, nmoves);

    int nlegal = 0;

    for (int i = 0; i < nmoves; ++i)
    {
        if (!ncPositionMakeMove(pos, moves[i]))
        {
            ncPositionUnmakeMove(pos);
            continue;
        }

        ++nlegal;

        int score = ncAlphaBetaQ(pos, depth - 1, -beta, -alpha, stop, nodes);

        ncPositionUnmakeMove(pos);

        if (score == NC_AB_INCOMPLETE)
            return NC_AB_INCOMPLETE;

        if (ncIntSyncRead(stop))
            return NC_AB_INCOMPLETE;

        score = -score;

        if (score > alpha)
            alpha = score;

        if (score >= beta)
            return score;
    }

    int value = alpha;

    if (!nlegal)
    {
        if (ncPositionIsCheck(pos))
            value = NC_AB_LOSS;
        else
            value = 0;
    }

    return value;
}

void ncAlphaBetaLoad(ncPosition* pos)
{
    pthread_mutex_lock(&search_mutex);
    search_position = *pos;
    char fen[100];
    ncPositionToFen(pos, fen, sizeof(fen));
    fprintf(stderr, "debug: loaded %s\n", fen);
    pthread_mutex_unlock(&search_mutex);
}
