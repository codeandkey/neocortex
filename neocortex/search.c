#include "search.h"
#include "mcts.h"

static int search_type = NC_SEARCH_DEFAULT;

int ncSearchType(int type)
{
    switch (type)
    {
        case NC_SEARCH_MCTS:
            break;
        default:
            return -1;
    }

    ncSearchStop();
    search_type = type;
    return 0;
}

void ncSearchStart(int nodes, int movetime, ncFnBestmove best_move, ncFnInfo info)
{
    if (search_type == NC_SEARCH_MCTS)
        ncMctsStart(nodes, movetime, best_move, info);
}

void ncSearchStop()
{
    if (search_type == NC_SEARCH_MCTS)
        ncMctsStop();
}

void ncSearchLoad(ncPosition* pos)
{
    if (search_type == NC_SEARCH_MCTS)
        ncMctsLoad(pos);
}
