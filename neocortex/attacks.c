/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "types.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

ncBitboard NC_KING_ATTACKS[64];
ncBitboard NC_KNIGHT_ATTACKS[64];
ncBitboard NC_PAWN_ATTACKS[2][64];
ncBitboard NC_PAWN_FRONTSPANS[2][64];
ncBitboard NC_PAWN_ATTACKSPANS[2][64];
ncBitboard* NC_ROOK_ATTACKS[64];
ncBitboard* NC_BISHOP_ATTACKS[64];

static ncBitboard make_rocc(int index, ncBitboard mask, int bits);

void ncAttacksInit() {
	// Zero tables
	memset(NC_KING_ATTACKS, 0, sizeof(NC_KING_ATTACKS));
	memset(NC_KNIGHT_ATTACKS, 0, sizeof(NC_KNIGHT_ATTACKS));
	memset(NC_PAWN_ATTACKS, 0, sizeof(NC_PAWN_ATTACKS));
	memset(NC_PAWN_FRONTSPANS, 0, sizeof(NC_PAWN_FRONTSPANS));
	memset(NC_PAWN_ATTACKSPANS, 0, sizeof(NC_PAWN_ATTACKSPANS));
	memset(NC_ROOK_ATTACKS, 0, sizeof(NC_ROOK_ATTACKS));
	memset(NC_BISHOP_ATTACKS, 0, sizeof(NC_BISHOP_ATTACKS));

	for (int sq = 0; sq < 64; ++sq) {
		int src_r = ncSquareRank(sq), src_f = ncSquareFile(sq);

		/* white pawn spans */
		for (int rr = src_r + 1; rr < 8; ++rr) {
			NC_PAWN_FRONTSPANS[NC_WHITE][sq] |= ncSquareMask(ncSquareAt(rr, src_f));

			if (src_f > 0) {
				NC_PAWN_ATTACKSPANS[NC_WHITE][sq] |= ncSquareMask(ncSquareAt(rr, src_f - 1));
			}

			if (src_f < 7) {
				NC_PAWN_ATTACKSPANS[NC_WHITE][sq] |= ncSquareMask(ncSquareAt(rr, src_f + 1));
			}
		}

		/* black pawn spans */
		for (int rr = src_r - 1; rr >= 0; --rr) {
			NC_PAWN_FRONTSPANS[NC_BLACK][sq] |= ncSquareMask(ncSquareAt(rr, src_f));

			if (src_f > 0) {
				NC_PAWN_ATTACKSPANS[NC_BLACK][sq] |= ncSquareMask(ncSquareAt(rr, src_f - 1));
			}

			if (src_f < 7) {
				NC_PAWN_ATTACKSPANS[NC_BLACK][sq] |= ncSquareMask(ncSquareAt(rr, src_f + 1));
			}
		}

		if (src_r < 7) {
			NC_PAWN_ATTACKS[NC_WHITE][sq] = ncBitboardShift(ncSquareMask(sq) & ~NC_FILE_A, NC_NORTHWEST) | ncBitboardShift(ncSquareMask(sq) & ~NC_FILE_H, NC_NORTHEAST);
		} else {
			NC_PAWN_ATTACKS[NC_WHITE][sq] = 0;
		}

		if (src_r > 0) {
			NC_PAWN_ATTACKS[NC_BLACK][sq] = ncBitboardShift(ncSquareMask(sq) & ~NC_FILE_A, NC_SOUTHWEST) | ncBitboardShift(ncSquareMask(sq) & ~NC_FILE_H, NC_SOUTHEAST);
		}
		else {
			NC_PAWN_ATTACKS[NC_BLACK][sq] = 0;
		}

		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_H), NC_EAST);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_A), NC_WEST);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_RANK_8), NC_NORTH);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_RANK_1), NC_SOUTH);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_H & ~NC_RANK_8), NC_NORTHEAST);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_A & ~NC_RANK_8), NC_NORTHWEST);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_H & ~NC_RANK_1), NC_SOUTHEAST);
		NC_KING_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_A & ~NC_RANK_1), NC_SOUTHWEST);

		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_H & ~NC_FILE_G & ~NC_RANK_8), NC_EAST * 2 + NC_NORTH);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_H & ~NC_FILE_G & ~NC_RANK_1), NC_EAST * 2 + NC_SOUTH);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_B & ~NC_FILE_A & ~NC_RANK_8), NC_WEST * 2 + NC_NORTH);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_FILE_B & ~NC_FILE_A & ~NC_RANK_1), NC_WEST * 2 + NC_SOUTH);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_RANK_1 & ~NC_RANK_2 & ~NC_FILE_A), NC_SOUTH * 2 + NC_WEST);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_RANK_1 & ~NC_RANK_2 & ~NC_FILE_H), NC_SOUTH * 2 + NC_EAST);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_RANK_7 & ~NC_RANK_8 & ~NC_FILE_A), NC_NORTH * 2 + NC_WEST);
		NC_KNIGHT_ATTACKS[sq] |= ncBitboardShift((ncSquareMask(sq) & ~NC_RANK_7 & ~NC_RANK_8 & ~NC_FILE_H), NC_NORTH * 2 + NC_EAST);

		int num_rook_roccs = 1 << NC_ROOK_BITS[sq];

		NC_ROOK_ATTACKS[sq] = (ncBitboard*) malloc(sizeof(ncBitboard) * num_rook_roccs);

		for (int i = 0; i < num_rook_roccs; ++i) {
			NC_ROOK_ATTACKS[sq][i] = 0;
		}

		for (int i = 0; i < num_rook_roccs; ++i) {
			ncBitboard rocc = make_rocc(i, NC_ROOK_MASKS[sq], NC_ROOK_BITS[sq]);
			int mindex = magic_index(rocc, NC_ROOK_MAGICS[sq], NC_ROOK_BITS[sq]);

			assert(!NC_ROOK_ATTACKS[sq][mindex]);

			ncBitboard* dst = &NC_ROOK_ATTACKS[sq][mindex];

			/* west */
			for (int f = src_f - 1; f >= 0; --f) {
				ncBitboard mask = ncSquareMask(ncSquareAt(src_r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* east */
			for (int f = src_f + 1; f < 8; ++f) {
				ncBitboard mask = ncSquareMask(ncSquareAt(src_r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* north */
			for (int r = src_r + 1; r < 8; ++r) {
				ncBitboard mask = ncSquareMask(ncSquareAt(r, src_f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* south */
			for (int r = src_r - 1; r >= 0; --r) {
				ncBitboard mask = ncSquareMask(ncSquareAt(r, src_f));
				*dst |= (ncBitboard) mask;
				if (rocc & mask) break;
			}
		}

		int num_bishop_roccs = 1 << NC_BISHOP_BITS[sq];

		NC_BISHOP_ATTACKS[sq] = (ncBitboard*) malloc(sizeof(ncBitboard) * num_bishop_roccs);

		for (int i = 0; i < num_bishop_roccs; ++i) {
			NC_BISHOP_ATTACKS[sq][i] = 0;
		}

		for (int i = 0; i < num_bishop_roccs; ++i) {
			ncBitboard rocc = make_rocc(i, NC_BISHOP_MASKS[sq], NC_BISHOP_BITS[sq]);
			int mindex = magic_index(rocc, NC_BISHOP_MAGICS[sq], NC_BISHOP_BITS[sq]);

			assert(!NC_BISHOP_ATTACKS[sq][mindex]);

			ncBitboard* dst = &NC_BISHOP_ATTACKS[sq][mindex];

			/* northwest */
			for (int f = src_f - 1, r = src_r + 1; f >= 0 && r < 8; --f, ++r) {
				ncBitboard mask = ncSquareMask(ncSquareAt(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* northeast */
			for (int f = src_f + 1, r = src_r + 1; f < 8 && r < 8; ++f, ++r) {
				ncBitboard mask = ncSquareMask(ncSquareAt(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* southeast */
			for (int f = src_f + 1, r = src_r - 1; f < 8 && r >= 0; ++f, --r) {
				ncBitboard mask = ncSquareMask(ncSquareAt(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* southwest */
			for (int f = src_f - 1, r = src_r - 1; f >= 0 && r >= 0; --f, --r) {
				ncBitboard mask = ncSquareMask(ncSquareAt(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}
		}
	}
}

ncBitboard make_rocc(int index, ncBitboard mask, int bits) {
	ncBitboard output = 0;

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
