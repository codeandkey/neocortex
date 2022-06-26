#include <stdlib.h>
#include <stdio.h>

#include "attacks.h"
#include "position.h"
#include "search.h"
#include "types.h"

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
    ncZobristInit();
    ncAttacksInit();
    ncBitboardInitBetween();

    ncPosition p;
    ncPositionInit(&p);

	//if (ncPositionFromFen(&p, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"))
	if (ncPositionFromFen(&p, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"))
		abort();

    int result;
    int nnodes = 5000;

    if (argc > 1)
        nnodes = strtol(argv[1], NULL, 10);

    while (!ncPositionIsTerminal(&p, &result))
    {
        ncMove move = ncSearch(&p, nnodes, 1.4f);

        char uci[6];
        ncMoveUCI(move, uci);

        printf("move %s\n", uci);

        ncPositionMakeMove(&p, move);

        char fen[100];
        ncPositionToFen(&p, fen, sizeof(fen));

        printf("%s\n", fen);

    }

    printf("result %d\n", result);
    return 0;
}
