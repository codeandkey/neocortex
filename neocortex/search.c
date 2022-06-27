#include "search.h"

#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int search_should_stop;
static int search_running;
static int search_iterations;
static int search_movetime;
static ncPosition search_position;
static ncFnBestmove search_cb_bestmove;
static ncFnInfo search_fn_info;
static pthread_mutex_t search_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t search_thread;

struct ncNode_t {
    int visits;
    float value;
    ncMove action;
    int action_col;
    struct ncNode_t* parent, *next, *child;
};

typedef struct ncNode_t ncNode;

void* ncSearchRun(void*);
void ncSearchSelect(ncPosition* pos, ncNode* root, ncNode* nodes, int* nnodes);

long milliseconds()
{
    struct timespec tm;
    timespec_get(&tm, TIME_UTC);
    return 1000 * (long) tm.tv_sec + (long) tm.tv_nsec / 1000000;
}

static inline void ncSearchBackprop(ncNode* root, float value)
{
    ++root->visits;
    root->value += 0.5 + ((float) root->action_col) * value / 2.0f;

    if (root->parent)
        ncSearchBackprop(root->parent, value);
}

void ncSearchStart(int nodes, int movetime, ncFnBestmove bm, ncFnInfo info)
{
    pthread_mutex_lock(&search_mutex);
    if (search_running)
    {
        fprintf(stderr, "error: ncSearchStart() called with search running\n");
        pthread_mutex_unlock(&search_mutex);
        return;
    }

    search_cb_bestmove = bm;
    search_fn_info = info;
    search_iterations = nodes;
    search_should_stop = 0;
    search_movetime = movetime;

    pthread_mutex_unlock(&search_mutex);
    pthread_create(&search_thread, NULL, ncSearchRun, NULL);
}

void ncSearchStop()
{
    pthread_mutex_lock(&search_mutex);

    if (!search_running)
    {
        pthread_mutex_unlock(&search_mutex);
        return;
    }

    search_should_stop = 1;
    pthread_mutex_unlock(&search_mutex);

    // Wait for search to terminate
    while (1)
    {
        pthread_mutex_lock(&search_mutex);

        if (!search_running)
        {
            search_should_stop = 0;
            pthread_mutex_unlock(&search_mutex);
            return;
        }

        pthread_mutex_unlock(&search_mutex);
    }
}

void* ncSearchRun(void* unused)
{
    // <iterations> is an upper bound on the number of new nodes.
    pthread_mutex_lock(&search_mutex);
    ncNode* nodes = (ncNode*) malloc(sizeof(ncNode) * search_iterations);
    int nnodes = 1;
    int iterations = search_iterations;
    ncPosition pos = search_position;
    search_should_stop = 0;
    search_running = 1;
    int movetime = search_movetime;
    pthread_mutex_unlock(&search_mutex);

    assert(nodes);

    char ff[100];
    ncPositionToFen(&pos, ff, sizeof(ff));

    printf("info string Searching %s\n", ff);

    // Initialize root node
    nodes[0].visits = 0;
    nodes[0].value = 0;
    nodes[0].action = NC_NULL;
    nodes[0].action_col = ncPositionGetCTM(&pos) * 2 - 1;
    nodes[0].parent = NULL;
    nodes[0].next = NULL;
    nodes[0].child = NULL;

    long starttime = milliseconds();
    long infotime = starttime;

    int lastnodes = 1;

    for (int i = 0; i < iterations - 1; ++i)
    {
        if (!(i % 1000))
        {
            if (movetime >= 0 && (milliseconds() - starttime >= movetime))
                break;

            pthread_mutex_lock(&search_mutex);
            if (search_should_stop)
            {
                pthread_mutex_unlock(&search_mutex);
                break;
            }

            pthread_mutex_unlock(&search_mutex);

            if (milliseconds() - infotime > NC_SEARCH_INFO_INTERVAL)
            {
                ncSearchInfo inf;
                inf.nodes = i;
                inf.elapsed = milliseconds() - starttime;
                inf.nps = (i - lastnodes) * 1000 / (milliseconds() - infotime);
                inf.ctm = ncPositionGetCTM(&pos);

                lastnodes = i;
                infotime = milliseconds();

                // Check current score
                int best_n = -1;

                for (ncNode* c = nodes->child; c; c = c->next)
                {
                    if (c->visits > best_n)
                    {
                        best_n = c->visits;
                        inf.score = c->value / c->visits;
                    }
                }

                if (search_fn_info)
                    search_fn_info(inf);
            }
        }

        ncSearchSelect(&pos, nodes, nodes, &nnodes);
        assert(nodes->child);
    }

    // Pick best move
    int best_n = -1;
    ncMove best_move = NC_NULL;

    for (ncNode* c = nodes->child; c; c = c->next)
    {
        if (c->visits > best_n)
        {
            best_n = c->visits;
            best_move = c->action;
        }
    }

    assert(ncMoveValid(best_move));
    free(nodes);

    if (search_cb_bestmove)
        search_cb_bestmove(best_move);

    pthread_mutex_lock(&search_mutex);
    search_running = 0;
    pthread_mutex_unlock(&search_mutex);

    return NULL;
}

void ncSearchSelect(ncPosition* pos, ncNode* root, ncNode* nodes, int* nnodes)
{
    ncMove pl_moves[NC_MAX_PL_MOVES];
    int num_pl_moves;

    // Check for terminal draws
    if (pos->ply[pos->nply - 1].halfmove_clock >= 50)
    {
        ncSearchBackprop(root, 0);
        return;
    }

    if (ncPositionRepCount(pos) >= 3)
    {
        ncSearchBackprop(root, 0);
        return;
    }

    // Terminal statemates and checkmates are detected during expansion.
    num_pl_moves = ncPositionPLMoves(pos, pl_moves);
    ncPositionOrderMoves(pos, pl_moves, num_pl_moves);

    ncNode* child = root->child;
    ncNode* best_child = child;
    ncNode* last_child = NULL;
    float best_uct = 0.0f;

    for (int i = 0; i < num_pl_moves; ++i)
    {
        if (!ncPositionMakeMove(pos, pl_moves[i]))
        {
            ncPositionUnmakeMove(pos);
            continue;
        }

        if (child)
        {
            float uct = child->value / child->visits + NC_EXPLORATION * sqrtf(log(root->visits) / (float) child->visits);

            if (uct > best_uct)
            {
                best_child = child;
                best_uct = uct;
            }

            last_child = child;
            child = child->next;

            ncPositionUnmakeMove(pos);
            continue;
        }

        // Expand this child.
        ncNode* newnode = &nodes[*nnodes];

        memset(newnode, 0, sizeof(ncNode));
        newnode->action_col = -root->action_col;
        newnode->action = pl_moves[i];
        newnode->parent = root;

        if (last_child)
            last_child->next = newnode;
        else
            root->child = newnode;

        *nnodes += 1;

        int score = ncPositionEvaluate(pos);
        float value = (score / (float) NC_SEARCH_EVAL_THRESHOLD);

        value = fmax(value, -NC_SEARCH_EVAL_MAX);
        value = fmin(value, NC_SEARCH_EVAL_MAX);

        ncSearchBackprop(newnode, value);
        ncPositionUnmakeMove(pos);
        return;
    }

    // If we've iterated through all the children, either none existed or
    // all children have been expanded.
    
    // If no children, then there are no legal moves in this position.
    if (!root->child)
    {
        if (ncPositionIsCheck(pos))
            ncSearchBackprop(root, root->action_col);
        else
            ncSearchBackprop(root, 0);

        return;
    }

    // Continue selecting through the most promising child.
    ncPositionMakeMove(pos, best_child->action);
    ncSearchSelect(pos, best_child, nodes, nnodes);
    ncPositionUnmakeMove(pos);
}

void ncSearchLoad(ncPosition* pos)
{
    pthread_mutex_lock(&search_mutex);
    search_position = *pos;
    char fen[100];
    ncPositionToFen(pos, fen, sizeof(fen));
    fprintf(stderr, "debug: loaded %s\n", fen);
    pthread_mutex_unlock(&search_mutex);
}
