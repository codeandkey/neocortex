#include "magic.h"
#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));

    nc3_magic_init();
    printf("Initialized magic tables.\n");

    printf("Here are some magic bitboard tests with random occs.");

    for (int t = 0; t < 1; ++t) {
        int sq = rand() % 64;
        printf("Test %d at source square %s:\n", t, nc3_square_tostr(sq));

        u64 occ = (((u64) (rand() & 0xFFFFFFFF)) << 32) | ((u64) (rand() & 0xFFFFFFFF));
        occ &= (((u64) (rand() & 0xFFFFFFFF)) << 32) | ((u64) (rand() & 0xFFFFFFFF));

        printf("Occupancy:\n%s", nc3_bitboard_tostr(occ));
        printf("Rook attacks:\n%s", nc3_bitboard_tostr(nc3_magic_query_rook_attacks(sq, occ)));

        printf("Move notation: ");
        const nc3_movelist* list = nc3_magic_query_rook_moves(sq ,occ);

        for (int i = 0; i < list->count; ++i) {
            printf("%d %s ", list->moves[i], nc3_move_tostr(list->moves[i]));
        }

        printf("\n");
    }

    nc3_magic_free();
    printf("Cleaned up magic tables.\n");

    return 0;
}
