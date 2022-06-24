#include <stdlib.h>
#include <stdio.h>

#include "attacks.h"
#include "position.h"

int perft(ncPosition* pos, int d)
{
    if (!d) return 1;

    ncMove moves[NC_MAX_PL_MOVES];
    int nmoves = ncPositionPLMoves(pos, moves);

    int total = 1;

    printf("%d PL moves:\n", nmoves);
    
    for (int i = 0; i < nmoves; ++i)
    {

        if (ncPositionMakeMove(pos, moves[i]))
        {
            printf("%d: legal move %c%c%c%c\n", i,
                ncSquareFile(ncMoveSrc(moves[i]))+'a',
                ncSquareRank(ncMoveSrc(moves[i]))+'1',
                ncSquareFile(ncMoveDst(moves[i]))+'a',
                ncSquareRank(ncMoveDst(moves[i]))+'1');
            total += perft(pos, d - 1);
        } else {
            printf("%d: illegal move %c%c%c%c\n", i,
                ncSquareFile(ncMoveSrc(moves[i]))+'a',
                ncSquareRank(ncMoveSrc(moves[i]))+'1',
                ncSquareFile(ncMoveDst(moves[i]))+'a',
                ncSquareRank(ncMoveDst(moves[i]))+'1');
        }

        ncPositionUnmakeMove(pos);
    }

    return total;
}

int main()
{
    ncZobristInit();
    ncAttacksInit();
    ncBitboardInitBetween();

    ncPosition p;
    ncPositionInit(&p);

    char fen[100];
    ncPositionToFen(&p, fen, sizeof(fen));

    printf("%s\n", fen);

    for (int d = 0; d < 6; ++d)
    {
        printf("perft %d: %d\n", d, perft(&p, d));
    }

    return 0;
}
