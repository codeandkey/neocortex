#include "search.h"

#include <math.h>
#include <stdlib.h>

struct ncNode_t {
    int visits;
    float value;
    ncMove action;
    int action_col;
    struct ncNode_t* parent, *next, *child;
};

typedef struct ncNode_t ncNode;

void ncSearchSelect(ncPosition* pos, ncNode* root, ncNode* nodes, int* nnodes, float expl);

static inline void ncSearchBackprop(ncNode* root, float value)
{
    ++root->visits;
    root->value += 0.5 + ((float) root->action_col) * value / 2.0f;

    if (root->parent)
        ncSearchBackprop(root->parent, value);
}

ncMove ncSearch(ncPosition* pos, int iterations, float expl)
{
    // <iterations> is an upper bound on the number of new nodes.
    ncNode* nodes = (ncNode*) malloc(sizeof(ncNode) * iterations);
    int nnodes = 1;

    assert(nodes);

    // Initialize root node
    nodes[0].visits = 0;
    nodes[0].value = 0;
    nodes[0].action = NC_NULL;
    nodes[0].action_col = ncPositionGetCTM(pos) * 2 - 1;
    nodes[0].parent = NULL;
    nodes[0].next = NULL;
    nodes[0].child = NULL;

    for (int i = 0; i < iterations - 1; ++i)
        ncSearchSelect(pos, nodes, nodes, &nnodes, expl);

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
    return best_move;
}

void ncSearchSelect(ncPosition* pos, ncNode* root, ncNode* nodes, int* nnodes, float expl)
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
            float uct = child->value / child->visits + expl * sqrtf(log(root->visits) / (float) child->visits);

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

        newnode->action_col = -root->action_col;
        newnode->action = pl_moves[i];
        newnode->visits = 0;
        newnode->value = 0;
        newnode->parent = root;
        newnode->child = NULL;
        newnode->next = NULL;

        if (last_child)
            last_child->next = newnode;
        else
            root->child = newnode;

        *nnodes += 1;

        int score = ncPositionEvaluate(pos);
        float value = (score / (float) NC_SEARCH_EVAL_THRESHOLD);

        value *= NC_SEARCH_EVAL_WEIGHT;

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
    ncSearchSelect(pos, best_child, nodes, nnodes, expl);
    ncPositionUnmakeMove(pos);
}
