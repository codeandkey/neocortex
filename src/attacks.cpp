/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "util.h"
#include "piece.h"
#include "square.h"
#include "color.h"
#include "type.h"

#include <cassert>

using namespace neocortex;

bitboard attacks::king_attacks[64] = { 0 };
bitboard attacks::knight_attacks[64] = { 0 };
bitboard attacks::pawn_attacks[2][64] = { {0} };
bitboard* attacks::rook_attacks[64] = { nullptr };
bitboard* attacks::bishop_attacks[64] = { nullptr };

static bitboard make_rocc(int index, bitboard mask, int bits);

void attacks::init() {
	for (int sq = 0; sq < 64; ++sq) {
		int src_r = square::rank(sq), src_f = square::file(sq);

		if (src_r < 7) {
			pawn_attacks[color::WHITE][sq] = bb::shift(bb::mask(sq) & ~FILE_A, NORTHWEST) | bb::shift(bb::mask(sq) & ~FILE_H, NORTHEAST);
		} else {
			pawn_attacks[color::WHITE][sq] = 0;
		}

		if (src_r > 0) {
			pawn_attacks[color::BLACK][sq] = bb::shift(bb::mask(sq) & ~FILE_A, SOUTHWEST) | bb::shift(bb::mask(sq) & ~FILE_H, SOUTHEAST);
		}
		else {
			pawn_attacks[color::BLACK][sq] = 0;
		}

		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_H), EAST);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_A), WEST);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~RANK_8), NORTH);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~RANK_1), SOUTH);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_H & ~RANK_8), NORTHEAST);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_A & ~RANK_8), NORTHWEST);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_H & ~RANK_1), SOUTHEAST);
		king_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_A & ~RANK_1), SOUTHWEST);

		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_H & ~FILE_G & ~RANK_8), EAST * 2 + NORTH);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_H & ~FILE_G & ~RANK_1), EAST * 2 + SOUTH);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_B & ~FILE_A & ~RANK_8), WEST * 2 + NORTH);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~FILE_B & ~FILE_A & ~RANK_1), WEST * 2 + SOUTH);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~RANK_1 & ~RANK_2 & ~FILE_A), SOUTH * 2 + WEST);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~RANK_1 & ~RANK_2 & ~FILE_H), SOUTH * 2 + EAST);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~RANK_7 & ~RANK_8 & ~FILE_A), NORTH * 2 + WEST);
		knight_attacks[sq] |= bb::shift((bb::mask(sq) & ~RANK_7 & ~RANK_8 & ~FILE_H), NORTH * 2 + EAST);

		int num_rook_roccs = 1 << magic::rook_bits[sq];

		rook_attacks[sq] = new bitboard[num_rook_roccs];

		for (int i = 0; i < num_rook_roccs; ++i) {
			rook_attacks[sq][i] = 0;
		}

		for (int i = 0; i < num_rook_roccs; ++i) {
			bitboard rocc = make_rocc(i, magic::rook_masks[sq], magic::rook_bits[sq]);
			int mindex = magic_index(rocc, magic::rook_magics[sq], magic::rook_bits[sq]);

			assert(!rook_attacks[sq][mindex]);

			bitboard* dst = &rook_attacks[sq][mindex];

			/* west */
			for (int f = src_f - 1; f >= 0; --f) {
				bitboard mask = bb::mask(square::at(src_r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* east */
			for (int f = src_f + 1; f < 8; ++f) {
				bitboard mask = bb::mask(square::at(src_r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* north */
			for (int r = src_r + 1; r < 8; ++r) {
				bitboard mask = bb::mask(square::at(r, src_f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* south */
			for (int r = src_r - 1; r >= 0; --r) {
				bitboard mask = bb::mask(square::at(r, src_f));
				*dst |= (bitboard) mask;
				if (rocc & mask) break;
			}
		}

		int num_bishop_roccs = 1 << magic::bishop_bits[sq];

		bishop_attacks[sq] = new bitboard[num_bishop_roccs];

		for (int i = 0; i < num_bishop_roccs; ++i) {
			bishop_attacks[sq][i] = 0;
		}

		for (int i = 0; i < num_bishop_roccs; ++i) {
			bitboard rocc = make_rocc(i, magic::bishop_masks[sq], magic::bishop_bits[sq]);
			int mindex = magic_index(rocc, magic::bishop_magics[sq], magic::bishop_bits[sq]);

			assert(!bishop_attacks[sq][mindex]);

			bitboard* dst = &bishop_attacks[sq][mindex];

			/* northwest */
			for (int f = src_f - 1, r = src_r + 1; f >= 0 && r < 8; --f, ++r) {
				bitboard mask = bb::mask(square::at(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* northeast */
			for (int f = src_f + 1, r = src_r + 1; f < 8 && r < 8; ++f, ++r) {
				bitboard mask = bb::mask(square::at(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* southeast */
			for (int f = src_f + 1, r = src_r - 1; f < 8 && r >= 0; ++f, --r) {
				bitboard mask = bb::mask(square::at(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}

			/* southwest */
			for (int f = src_f - 1, r = src_r - 1; f >= 0 && r >= 0; --f, --r) {
				bitboard mask = bb::mask(square::at(r, f));
				*dst |= mask;
				if (rocc & mask) break;
			}
		}
	}
}

bitboard make_rocc(int index, bitboard mask, int bits) {
	bitboard output = 0;

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
