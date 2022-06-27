#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "attacks.h"
#include "position.h"
#include "search.h"
#include "types.h"
#include "uci.h"

typedef struct {
    int total;
    int captures;
    int castles;
    int checks;
    int ep;
} perft_state;

void perft(ncPosition* pos, int d, perft_state* state)
{
    if (!d)
    {
        ++state->total;
        state->captures += ncPositionIsCapture(pos);
        state->castles += ncPositionIsCastle(pos);
        state->checks += ncPositionIsCheck(pos);
        state->ep += ncPositionIsEP(pos);
        return;
    }

    ncMove moves[NC_MAX_PL_MOVES];
    int nmoves = ncPositionPLMoves(pos, moves);

    for (int i = 0; i < nmoves; ++i)
    {
        if (ncPositionMakeMove(pos, moves[i]))
            perft(pos, d - 1, state);

        ncPositionUnmakeMove(pos);
    }
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    ncZobristInit();
    ncAttacksInit();
    ncBitboardInitBetween();

    ncPosition p;
    ncPositionInit(&p);

    return ncUciStart();
}
