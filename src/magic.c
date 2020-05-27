#include "magic.h"
#include "magic_consts.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

/*
 * Global attack mask and move database lookups.
 * Has to be initialized at runtime.
 */

static struct {
	nc_bb* rook_attack_masks[64];
	nc_bb* bishop_attack_masks[64];
} nc_magic_database;

static int _nc_magic_index(nc_bb occ, nc_bb magic, int bits);
static nc_bb _nc_make_relevant_occ(int index, nc_bb mask, int bits);

void nc_magic_init() {
	/* 
	 * We have the magic numbers, the relevant masks, and the bits to shift but still need to
	 * generate the attack masks and move lists for each magic index.
	 *
	 * Theoretically this could be stuffed into static data but it would make this
	 * source code really nasty.
	 */

	/* Allocate movelists and attack lists. */
	for (int sq = 0; sq < 64; ++sq) {
		int num_rook_occs = (1 << _nc_rook_bits[sq]);
		int num_bishop_occs = (1 << _nc_bishop_bits[sq]);

		nc_magic_database.rook_attack_masks[sq] = calloc(num_rook_occs, sizeof **nc_magic_database.rook_attack_masks);
		nc_magic_database.bishop_attack_masks[sq] = calloc(num_bishop_occs, sizeof **nc_magic_database.bishop_attack_masks);
	}

	/*
	 * Generate rook attacks
	 */

	for (int sq = 0; sq < 64; ++sq) {
		/*
		 * Walk through potential relevant occupancies.
		 * The number of bits and the mask is known.
		 */

		int num_rook_bits = _nc_rook_bits[sq];
		int num_rook_occs = (1 << num_rook_bits);
		nc_bb rook_rocc_mask = _nc_rook_masks[sq];
		nc_bb rook_magic = _nc_rook_magics[sq];

		for (int occ_index = 0; occ_index < num_rook_occs; ++occ_index) {
			nc_bb relevant_occ = _nc_make_relevant_occ(occ_index, rook_rocc_mask, num_rook_bits);

			/* Find where the magic takes us with the occupancy. */
			int magic_index = _nc_magic_index(relevant_occ, rook_magic, num_rook_bits);

			if (nc_magic_database.rook_attack_masks[sq][magic_index]) {
				nc_abort("Rook magic lookup collision on key %d", magic_index);
			}

			/* Walk in 4 directions, adding bits to the attack mask and moves to the movelist. */
			nc_square cur;

			/* East */
			for (int ar = nc_square_rank(sq), af = nc_square_file(sq) + 1; af <= 7; ++af) {
				cur = nc_square_at(ar, af);
				nc_magic_database.rook_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}

			/* West */
			for (int ar = nc_square_rank(sq), af = nc_square_file(sq) - 1; af >= 0; --af) {
				cur = nc_square_at(ar, af);
				nc_magic_database.rook_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}

			/* North */
			for (int ar = nc_square_rank(sq) + 1, af = nc_square_file(sq); ar <= 7; ++ar) {
				cur = nc_square_at(ar, af);
				nc_magic_database.rook_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}

			/* West */
			for (int ar = nc_square_rank(sq) - 1, af = nc_square_file(sq); ar >= 0; --ar) {
				cur = nc_square_at(ar, af);
				nc_magic_database.rook_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}
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

		int num_bishop_bits = _nc_bishop_bits[sq];
		int num_bishop_occs = (1 << num_bishop_bits);
		nc_bb bishop_rocc_mask = _nc_bishop_masks[sq];
		nc_bb bishop_magic = _nc_bishop_magics[sq];

		for (int occ_index = 0; occ_index < num_bishop_occs; ++occ_index) {
			nc_bb relevant_occ = _nc_make_relevant_occ(occ_index, bishop_rocc_mask, num_bishop_bits);

			/* Find where the magic takes us with the occupancy. */
			int magic_index = _nc_magic_index(relevant_occ, bishop_magic, num_bishop_bits);

			if (nc_magic_database.bishop_attack_masks[sq][magic_index]) {
				nc_abort("Bishop magic lookup collision on key %d", magic_index);
			}

			/* Walk in 4 directions, adding bits to the attack mask and moves to the movelist. */
			nc_square cur;

			/* Northeast */
			for (int ar = nc_square_rank(sq) + 1, af = nc_square_file(sq) + 1; af <= 7 && ar <= 7; ++af, ++ar) {
				cur = nc_square_at(ar, af);
				nc_magic_database.bishop_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}

			/* Northwest */
			for (int ar = nc_square_rank(sq) + 1, af = nc_square_file(sq) - 1; af >= 0 && ar <= 7; --af, ++ar) {
				cur = nc_square_at(ar, af);
				nc_magic_database.bishop_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}

			/* Southeast */
			for (int ar = nc_square_rank(sq) - 1, af = nc_square_file(sq) + 1; af <= 7 && ar >= 0; ++af, --ar) {
				cur = nc_square_at(ar, af);
				nc_magic_database.bishop_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}

			/* Northwest */
			for (int ar = nc_square_rank(sq) - 1, af = nc_square_file(sq) - 1; af >= 0 && ar >= 0; --af, --ar) {
				cur = nc_square_at(ar, af);
				nc_magic_database.bishop_attack_masks[sq][magic_index] |= nc_bb_mask(cur);
				if (relevant_occ & nc_bb_mask(cur)) break;
			}
		}
	}

	nc_debug("Initialized magic bitboard attack lookups.");
}

void nc_magic_free() {
	for (int sq = 0; sq < 64; ++sq) {
		free(nc_magic_database.rook_attack_masks[sq]);
		free(nc_magic_database.bishop_attack_masks[sq]);
	}

	nc_debug("Cleaned up magic bitboard lookups.");
}

nc_bb nc_magic_query_rook_attacks(nc_square sq, nc_bb occ) {
	int magic = _nc_magic_index(occ & _nc_rook_masks[sq], _nc_rook_magics[sq], _nc_rook_bits[sq]);
	return nc_magic_database.rook_attack_masks[sq][magic];
}

nc_bb nc_magic_query_bishop_attacks(nc_square sq, nc_bb occ) {
	int magic = _nc_magic_index(occ & _nc_bishop_masks[sq], _nc_bishop_magics[sq], _nc_bishop_bits[sq]);
	return nc_magic_database.bishop_attack_masks[sq][magic];
}

int _nc_magic_index(nc_bb masked_occ, nc_bb magic, int bits) {
	return (int)((masked_occ * magic) >> (64 - bits));
}

nc_bb _nc_make_relevant_occ(int index, nc_bb mask, int bits) {
	/* The bits of index need to be wrapped in a one-to-one manner into the mask's place. */
	/* This is definitely NOT optimal at all. But it is correct and only called on startup. */

	nc_bb output = 0;

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
