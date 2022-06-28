#include "mcts.h"

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

#define UNKNOWN -3
#define FULL -2

struct ncNode_t {
    int visits;
    float value;
    ncMove action;
    int action_col;
    int cache;
    struct ncNode_t* parent, *next, *child;
};

typedef struct ncNode_t ncNode;

typedef struct {
    pthread_t handle;
    pthread_mutex_t lock;
    int running;
    int nodes;
    int should_stop;
    int depth;
    float score;
    ncPosition pos;
    ncNode* tree;
    int treelen;
    ncMove best_move;
} ncSearchWorker;

static ncSearchWorker search_workers[NC_SEARCH_WORKERS_MAX];
static int num_search_workers = NC_SEARCH_WORKERS_DEFAULT;
static int search_tree_size = NC_MCTS_TREESIZE;

void* ncMctsRun(void*);
int ncMctsSelect(ncPosition* pos, ncNode* root, ncNode* tree, int* nodes);

long milliseconds()
{
    struct timespec tm;
    timespec_get(&tm, TIME_UTC);
    return 1000 * (long) tm.tv_sec + (long) tm.tv_nsec / 1000000;
}

static inline void ncMctsBackprop(ncNode* root, float value)
{
    ++root->visits;
    root->value += 0.5 + ((float) root->action_col) * value / 2.0f;

    if (root->parent)
        ncMctsBackprop(root->parent, value);
}

void ncMctsStart(int nodes, int movetime, ncFnBestmove bm, ncFnInfo info)
{
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
    search_should_stop = 0;
    search_movetime = movetime;

    pthread_mutex_unlock(&search_mutex);
    pthread_create(&search_thread, NULL, ncMctsRun, NULL);
}

void ncMctsStop()
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

void* ncMctsWorkerMain(void* w)
{
    ncSearchWorker* self = (ncSearchWorker*) w;

    pthread_mutex_lock(&self->lock);
    self->should_stop = 0;
    self->running = 1;
    self->nodes = 0;
    self->score = 0;
    self->depth = 0;
    self->treelen = 1;
    pthread_mutex_unlock(&self->lock);

    self->tree = (ncNode*) malloc(sizeof(ncNode) * NC_MCTS_TREESIZE);

    memset(&self->tree[0], 0, sizeof(self->tree[0]));
    self->tree[0].action_col = ncPositionGetCTM(&self->pos) * 2 - 1;
    self->tree[0].action = NC_NULL;
    self->tree[0].cache = UNKNOWN;

    for (int i = 0;; ++i)
    {
        pthread_mutex_lock(&self->lock);

        if (self->should_stop)
        {
            self->running = 0;
            pthread_mutex_unlock(&self->lock);
            break;
        }

        ++self->nodes;

        pthread_mutex_unlock(&self->lock);

        int depth = ncMctsSelect(&self->pos, self->tree, self->tree, &self->treelen);

        if (depth > self->depth)
        {
            pthread_mutex_lock(&self->lock);
            self->depth = depth;
            pthread_mutex_unlock(&self->lock);
        }

        if (i && !(i % 10000))
        {
            int best_n = 0;
            ncNode* best_child;

            for (ncNode* c = self->tree->child; c; c = c->next)
            {
                if (c->visits > best_n)
                {
                    best_n = c->visits;
                    best_child = c;
                }
            }

            assert(best_child);

            self->score = best_child->value / best_child->visits;
            self->best_move = best_child->action;
        }
    }

    int best_n = 0;
    ncNode* best_child;

    for (ncNode* c = self->tree->child; c; c = c->next)
    {
        if (c->visits > best_n)
        {
            best_n = c->visits;
            best_child = c;
        }
    }

    assert(best_child);

    self->score = best_child->value / best_child->visits;
    self->best_move = best_child->action;

    free(self->tree);

    return NULL;
}

void* ncMctsRun(void* unused)
{
    ncMctsStop();

    // <iterations> is an upper bound on the number of new nodes.
    pthread_mutex_lock(&search_mutex);
    ncPosition pos = search_position;
    search_should_stop = 0;
    search_running = 1;

    int movetime = search_movetime;
    int maxnodes = search_iterations;

    pthread_mutex_unlock(&search_mutex);

    char ff[100];
    ncPositionToFen(&pos, ff, sizeof(ff));

    printf("info string Searching %s\n", ff);

    long starttime = milliseconds();
    long infotime = starttime;
    long ttime = starttime;

    int lastnodes = 1;

    // Spin up workers
    for (int i = 0; i < num_search_workers; ++i)
    {
        search_workers[i].pos = pos;
        pthread_mutex_init(&search_workers[i].lock, NULL);
        pthread_create(&search_workers[i].handle, NULL, ncMctsWorkerMain, &search_workers[i]);
    }

    while (1)
    {
        if (milliseconds() - ttime < NC_MCTS_TEST_INTERVAL)
            continue;

        // Compute totals
        long total_nodes = 0;
        int maxdepth = 0;
        float avg_score = 0;

        for (int i = 0; i < num_search_workers; ++i)
        {
            pthread_mutex_lock(&search_workers[i].lock);
            total_nodes += search_workers[i].nodes;
            avg_score += search_workers[i].score;

            if (search_workers[i].depth > maxdepth)
                maxdepth = search_workers[i].depth;

            pthread_mutex_unlock(&search_workers[i].lock);
        }

        avg_score /= num_search_workers;

        if (search_iterations > 0 && total_nodes >= search_iterations)
            break;

        if (movetime >= 0 && (milliseconds() - starttime >= movetime))
            break;

        pthread_mutex_lock(&search_mutex);
        if (search_should_stop)
        {
            pthread_mutex_unlock(&search_mutex);
            break;
        }
        pthread_mutex_unlock(&search_mutex);

        if (milliseconds() - infotime > NC_MCTS_INFO_INTERVAL)
        {
            ncSearchInfo inf;
            inf.nodes = total_nodes;
            inf.elapsed = milliseconds() - starttime;
            inf.nps = (1000 * (long) (inf.nodes - lastnodes)) / (long) (milliseconds() - infotime);
            inf.ctm = ncPositionGetCTM(&pos);
            inf.depth = maxdepth;

            inf.score = (int) (avg_score * 1000.0f - 500.0f);
            inf.mate_score = 0;

            lastnodes = inf.nodes;
            infotime = milliseconds();

            if (search_fn_info)
                search_fn_info(inf);
        }
    }
    
    // Stop workers
    for (int i = 0; i < num_search_workers; ++i)
    {
        pthread_mutex_lock(&search_workers[i].lock);
        search_workers[i].should_stop = 1;
        pthread_mutex_unlock(&search_workers[i].lock);
    }

    for (int i = 0; i < num_search_workers; ++i)
    {
        pthread_join(search_workers[i].handle, NULL);
        pthread_mutex_destroy(&search_workers[i].lock);
    }

    // Pick best move
    ncMove best_move = search_workers[rand() % num_search_workers].best_move;

    assert(ncMoveValid(best_move));

    if (search_cb_bestmove)
        search_cb_bestmove(best_move);

    pthread_mutex_lock(&search_mutex);
    search_running = 0;
    pthread_mutex_unlock(&search_mutex);

    return NULL;
}

int ncMctsSelect(ncPosition* pos, ncNode* root, ncNode* tree, int* nnodes)
{
    // We first check if the node has a cached value or is complete.
    ncNode* child = root->child;

    if (root->cache != UNKNOWN)
    {
        if (root->cache != FULL)
        {
            ncMctsBackprop(root, root->cache);
            return 0;
        }

        // This node is known to be complete (and nonterminal). We can skip
        // movegen and just pick the child with best UCT.
        //
        // At this point we know the child structure of root will not
        // be modified, so we do not need to synchronize.
        
        ncNode* best_child = child;
        int best_action;

        float best_uct = 0.0f;

        while (child)
        {
            float uct = child->value / child->visits + NC_MCTS_EXPLORATION * sqrtf(log(root->visits) / (float) child->visits);

            if (uct > best_uct)
            {
                best_child = child;
                best_uct = uct;
                best_action = child->action;
            }

            child = child->next;
        }

        // Continue selecting through the most promising child.
        ncPositionMakeMove(pos, best_action);
        int retdepth = 1 + ncMctsSelect(pos, best_child, tree, nnodes);
        ncPositionUnmakeMove(pos);

        return retdepth;
    }

    // Check for terminal draws
    if (pos->ply[pos->nply - 1].halfmove_clock >= 50)
    {
        root->cache = 0;
        ncMctsBackprop(root, 0);
        return 0;
    }

    if (ncPositionRepCount(pos) >= 3)
    {
        root->cache = 0;
        ncMctsBackprop(root, 0);
        return 0;
    }

    // This node is not known to complete and could be a terminal or could
    // require a child to be expanded.
    // We do not need to track UCT.

    ncNode* last_child = NULL;
    ncMove pl_moves[NC_MAX_PL_MOVES];
    int num_pl_moves;

    num_pl_moves = ncPositionPLMoves(pos, pl_moves);
    //ncPositionOrderMoves(pos, pl_moves, num_pl_moves); // why order moves? FPU is infinite anyway

    for (int i = 0; i < num_pl_moves; ++i)
    {
        if (!ncPositionMakeMove(pos, pl_moves[i]))
        {
            ncPositionUnmakeMove(pos);
            continue;
        }

        if (child)
        {
            // We can unmake this move and continue, there is already a child.
            last_child = child;
            child = child->next;

            ncPositionUnmakeMove(pos);
            continue;
        }

        // We will expand this child.
        child = &tree[*nnodes];

        *nnodes += 1;

        if (*nnodes >= search_tree_size)
        {
            fprintf(stderr, "ERROR: node limit reached %d\n", search_tree_size);
            abort();
        }

        memset(child, 0, sizeof(ncNode));

        child->action_col = -root->action_col;
        child->action = pl_moves[i];
        child->parent = root;
        child->cache = UNKNOWN;

        if (last_child)
            last_child->next = child;
        else
            root->child = child;

        int score = ncPositionEvaluate(pos);
        float value = (score / (float) NC_MCTS_EVAL_THRESHOLD);

        // Create makeshift normal distribution
        float noise = 0;

        noise += ((float) rand() / (float) RAND_MAX);
        noise += ((float) rand() / (float) RAND_MAX);
        noise += ((float) rand() / (float) RAND_MAX);
        noise += ((float) rand() / (float) RAND_MAX);
        noise += ((float) rand() / (float) RAND_MAX);
        noise += ((float) rand() / (float) RAND_MAX);

        noise -= 6.0f;
        value += (noise / 6.0f) * NC_MCTS_NOISE;

        value = fmax(value, -NC_MCTS_EVAL_MAX);
        value = fmin(value, NC_MCTS_EVAL_MAX);

        ncPositionUnmakeMove(pos);
        ncMctsBackprop(child, value);

        return 0;
    }

    // At this point the node is either detected to be complete or terminal.
    
    // If no children, then there are no legal moves in this position.
    if (!root->child)
    {
        if (ncPositionIsCheck(pos))
        {
            root->cache = root->action_col;
            ncMctsBackprop(root, root->action_col);
        }
        else
        {
            root->cache = 0;
            ncMctsBackprop(root, 0);
        }

        return 0;
    }

    // Otherwise, this node is known to be full. We can simply reselect at this
    // node to pull from the cache.
    root->cache = FULL;
    return 1 + ncMctsSelect(pos, root, tree, nnodes);
}

void ncMctsLoad(ncPosition* pos)
{
    pthread_mutex_lock(&search_mutex);
    search_position = *pos;
    char fen[100];
    ncPositionToFen(pos, fen, sizeof(fen));
    fprintf(stderr, "debug: loaded %s\n", fen);
    pthread_mutex_unlock(&search_mutex);
}
