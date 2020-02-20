#include "magic.h"
#include "magic_consts.h"
#include "types.h"

#include <stdlib.h>
#include <stdio.h>

/*
 * Global attack mask and move database lookups.
 * Has to be initialized at runtime.
 */

static struct _nc3_magic_database {
    u64* rook_attack_masks[64];
    u64* bishop_attack_masks[64];
    nc3_movelist* rook_move_lists[64];
    nc3_movelist* bishop_move_lists[64];
} nc3_magic_database;

static int _nc3_magic_index(u64 occ, u64 magic, int bits);
static u64 _nc3_make_relevant_occ(int index, u64 mask, int bits);

void nc3_magic_init() {
    /* 
     * We have the magic numbers, the relevant masks, and the bits to shift but still need to
     * generate the attack masks and move lists for each magic index.
     *
     * Theoretically this could be stuffed into static data but it would make this
     * source code really nasty.
     */

    printf("ROOK ROCC MASKS");
    for (int sq = 0; sq < 64; ++sq) {
        printf("ROOK on %s:\n%s", nc3_square_tostr(sq), nc3_bitboard_tostr(_nc3_rook_masks[sq]));
    }

    /* Allocate movelists and attack lists. */
    for (int sq = 0; sq < 64; ++sq) {
        int num_rook_occs = (1 << _nc3_rook_bits[sq]);
        int num_bishop_occs = (1 << _nc3_bishop_bits[sq]);

        nc3_magic_database.rook_attack_masks[sq] = calloc(num_rook_occs, sizeof **nc3_magic_database.rook_attack_masks);
        nc3_magic_database.bishop_attack_masks[sq] = calloc(num_bishop_occs, sizeof **nc3_magic_database.bishop_attack_masks);
        nc3_magic_database.rook_move_lists[sq] = calloc(num_rook_occs, sizeof **nc3_magic_database.rook_move_lists);
        nc3_magic_database.bishop_move_lists[sq] = calloc(num_bishop_occs, sizeof **nc3_magic_database.bishop_move_lists);

        for (int i = 0; i < num_rook_occs; ++i) {
            nc3_movelist_init(nc3_magic_database.rook_move_lists[sq] + i, 0);
        }

        for (int i = 0; i < num_bishop_occs; ++i) {
            nc3_movelist_init(nc3_magic_database.bishop_move_lists[sq] + i, 0);
        }
    }

    /*
     * Generate rook attacks
     */

    for (int sq = 0; sq < 64; ++sq) {
        /*
         * Walk through potential relevant occupancies.
         * The number of bits and the mask is known.
         */

        int num_rook_bits = _nc3_rook_bits[sq];
        int num_rook_occs = (1 << num_rook_bits);
        u64 rook_rocc_mask = _nc3_rook_masks[sq];
        u64 rook_magic = _nc3_rook_magics[sq];

        for (int occ_index = 0; occ_index < num_rook_occs; ++occ_index) {
            u64 relevant_occ = _nc3_make_relevant_occ(occ_index, rook_rocc_mask, num_rook_bits);

            //if (sq == NC3_SQAT(6, 0)) printf("generating rook magics for relevant occ\n%s", nc3_bitboard_tostr(relevant_occ));

            /* Find where the magic takes us with the occupancy. */
            int magic_index = _nc3_magic_index(relevant_occ, rook_magic, num_rook_bits);

            if (sq == NC3_SQAT(6, 0) && magic_index == 512) {
                printf("landed on magic index 512 on square a7\n");
                printf("..rook_rocc_mask:\n%s", nc3_bitboard_tostr(rook_rocc_mask));
                printf("..relevant_occ:\n%s", nc3_bitboard_tostr(relevant_occ));
                printf("..relevant_occ:%lx\n", relevant_occ);
                printf("..magic multiplicand: %lx\n", rook_magic);
                printf("..product: %lx\n", rook_magic * relevant_occ);
                printf("..shifted: %d\n", (int)((rook_magic * relevant_occ) >> (64 - num_rook_bits)));
                printf("..num bits:%d\n", num_rook_bits);
            }

            /* Walk in 4 directions, adding bits to the attack mask and moves to the movelist. */
            nc3_square cur;

            /* East */
            for (int ar = NC3_RANK(sq), af = NC3_FILE(sq) + 1; af <= 7; ++af) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.rook_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.rook_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /* West */
            for (int ar = NC3_RANK(sq), af = NC3_FILE(sq) - 1; af >= 0; --af) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.rook_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.rook_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /* North */
            for (int ar = NC3_RANK(sq) + 1, af = NC3_FILE(sq); ar <= 7; ++ar) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.rook_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.rook_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /* West */
            for (int ar = NC3_RANK(sq) - 1, af = NC3_FILE(sq); ar >= 0; --ar) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.rook_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.rook_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /*if (sq == NC3_SQAT(6, 0) && (relevant_occ == (NC3_SQMASK(NC3_SQAT(1, 0)) | NC3_SQMASK(NC3_SQAT(6, 6))) || magic_index == 512)) {
                printf("generated rook attacks\n%s", nc3_bitboard_tostr(nc3_magic_database.rook_attack_masks[sq][magic_index]));
                printf("..for relevant occupancy\n%s", nc3_bitboard_tostr(relevant_occ));
                printf("..with relevant mask\n%s", nc3_bitboard_tostr(rook_rocc_mask));
                printf("..magic ind %d\n", magic_index);
            }*/
        }
    }

    /*
     * Generate bishop attacks
     */

    for (int sq = 0; sq < 64; ++sq) {
        /*
         * Walk through potential relevant occupancies.
         * The number of bits and the mask is known.
         */

        int num_bishop_bits = _nc3_bishop_bits[sq];
        int num_bishop_occs = (1 << num_bishop_bits);
        u64 bishop_rocc_mask = _nc3_bishop_masks[sq];
        u64 bishop_magic = _nc3_bishop_magics[sq];

        for (int occ_index = 0; occ_index < num_bishop_occs; ++occ_index) {
            u64 relevant_occ = _nc3_make_relevant_occ(occ_index, bishop_rocc_mask, num_bishop_bits);

            /* Find where the magic takes us with the occupancy. */
            int magic_index = _nc3_magic_index(relevant_occ, bishop_magic, num_bishop_bits);

            /* Walk in 4 directions, adding bits to the attack mask and moves to the movelist. */
            nc3_square cur;

            /* Northeast */
            for (int ar = NC3_RANK(sq) + 1, af = NC3_FILE(sq) + 1; af <= 7 && ar <= 7; ++af, ++ar) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.bishop_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.bishop_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /* Northwest */
            for (int ar = NC3_RANK(sq) + 1, af = NC3_FILE(sq) - 1; af >= 0 && ar <= 7; --af, ++ar) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.bishop_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.bishop_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /* Southeast */
            for (int ar = NC3_RANK(sq) - 1, af = NC3_FILE(sq) + 1; af <= 7 && ar >= 0; ++af, --ar) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.bishop_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.bishop_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }

            /* Northwest */
            for (int ar = NC3_RANK(sq) - 1, af = NC3_FILE(sq) - 1; af >= 0 && ar >= 0; --af, --ar) {
                cur = NC3_SQAT(ar, af);
                nc3_movelist_add(nc3_magic_database.bishop_move_lists[sq] + magic_index, NC3_MOVE(sq, cur));
                nc3_magic_database.bishop_attack_masks[sq][magic_index] |= NC3_SQMASK(cur);
                if (relevant_occ & NC3_SQMASK(cur)) break;
            }
        }
    }
}

void nc3_magic_free() {
    for (int sq = 0; sq < 64; ++sq) {
        free(nc3_magic_database.rook_move_lists[sq]);
        free(nc3_magic_database.bishop_move_lists[sq]);
        free(nc3_magic_database.rook_attack_masks[sq]);
        free(nc3_magic_database.bishop_attack_masks[sq]);
    }
}

const nc3_movelist* nc3_magic_query_rook_moves(nc3_square sq, u64 occ) {
    int magic = _nc3_magic_index(occ & _nc3_rook_masks[sq], _nc3_rook_magics[sq], _nc3_rook_bits[sq]);
    return nc3_magic_database.rook_move_lists[sq] + magic;
}

const nc3_movelist* nc3_magic_query_bishop_moves(nc3_square sq, u64 occ) {
    int magic = _nc3_magic_index(occ & _nc3_bishop_masks[sq], _nc3_bishop_magics[sq], _nc3_bishop_bits[sq]);
    return nc3_magic_database.bishop_move_lists[sq] + magic;
}

u64 nc3_magic_query_rook_attacks(nc3_square sq, u64 occ) {
    printf("querying rook attack, rocc=\n%s", nc3_bitboard_tostr(occ & _nc3_rook_masks[sq]));
    int magic = _nc3_magic_index(occ & _nc3_rook_masks[sq], _nc3_rook_magics[sq], _nc3_rook_bits[sq]);
    return nc3_magic_database.rook_attack_masks[sq][magic];
}

u64 nc3_magic_query_bishop_attacks(nc3_square sq, u64 occ) {
    int magic = _nc3_magic_index(occ & _nc3_bishop_masks[sq], _nc3_bishop_magics[sq], _nc3_bishop_bits[sq]);
    return nc3_magic_database.bishop_attack_masks[sq][magic];
}

int _nc3_magic_index(u64 masked_occ, u64 magic, int bits) {
    return (int)((masked_occ * magic) >> (64 - bits));
}

u64 _nc3_make_relevant_occ(int index, u64 mask, int bits) {
    /* The bits of index need to be wrapped in a one-to-one manner into the mask's place. */
    /* This is definitely NOT optimal at all. But it is correct and only called on startup. */

    u64 output = 0;

    for (int i = 0; i < bits; ++i) {
        if ((index >> i) & 1) {
            /* The ith bit in mask should be passed to result. */

            int currentbit = -1;
            for (int maskpos = 0; maskpos < 64; ++maskpos) {
                if ((mask >> maskpos) & 1) {
                    if (++currentbit == i) {
                        /* Pass this bit on! */
                        output |= (1ULL << maskpos);
                    }
                }
            }
        }
    }

    return output;
}
