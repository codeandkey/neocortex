#include "types.h"
#include "attacks.h"

#include <string.h>

ncBitboard NC_BETWEEN[64][64];
static int between_init = 0;

void ncBitboardInitBetween()
{
    memset(NC_BETWEEN, 0, sizeof NC_BETWEEN);

    for (ncSquare src = 0; src < 64; ++src)
    {
        ncBitboard dstsqs = ncAttacksQueen(src, 0ULL);

        while (dstsqs)
        {
            ncSquare dst = ncBitboardPop(&dstsqs);
			ncBitboard between = 0ULL;

            ncSquare start = src;

            while (start != dst)
            {
                int shift = 0;

                if (ncSquareRank(start) < ncSquareRank(dst))
                    shift += NC_NORTH;
                else if (ncSquareRank(start) > ncSquareRank(dst))
                    shift += NC_SOUTH;

                if (ncSquareFile(start) < ncSquareFile(dst))
                    shift += NC_EAST;
                else if (ncSquareFile(start) > ncSquareFile(dst))
                    shift += NC_WEST;

                start += shift;
                between |= ncSquareMask(start);
            }

            NC_BETWEEN[src][dst] = between & ~ncSquareMask(dst);
        }
    }

    between_init = 1;
}

ncBitboard ncBitboardBetween(ncSquare src, ncSquare dst)
{
    assert(ncSquareValid(src));
    assert(ncSquareValid(dst));
    assert(between_init);

    return NC_BETWEEN[src][dst];
}
