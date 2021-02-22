/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "bitboard.h"
#include "magic_consts.h"

#include <cassert>

namespace neocortex {
	namespace attacks {
		extern bitboard king_attacks[64], knight_attacks[64], pawn_attacks[2][64];
		extern bitboard* rook_attacks[64], *bishop_attacks[64];

		/**
		 * Initializes the attack lookup tables.
		 * Must be called before performing any lookups.
		 * Failure to do so will result in undefined behavior.
		 */
		void init();

		/**
		 * Computes a magic attack index from relevant occupancy, magic number and bits.
		 * Used internally for sliding piece attack lookups.
		 *
		 * @param rocc Relevant occupancy from sliding piece.
		 * @param magic Magic number. (see magic_consts)
		 * @param bits Number if relevant occupancy bits.
		 *
		 * @return Attack index.
		 */
		inline int magic_index(bitboard rocc, bitboard magic, int bits) {
			return (int) ((rocc * magic) >> (64 - bits));
		}

		/**
		 * Looks up pawn attacks from a source square.
		 *
		 * @param col Attacking color.
		 * @param sq Source square.
		 * @return Bitboard of attacked squares.
		 */
		inline bitboard pawn(int col, int sq) {
			return pawn_attacks[col][sq];
		}

		/**
		 * Looks up king attacks from a source square.
		 *
		 * @param sq Source square.
		 * @return Bitboard of attacked squares.
		 */
		inline bitboard king(int sq) {
			return king_attacks[sq];
		}

		/**
		 * Looks up knight attacks from a source square.
		 *
		 * @param sq Source square.
		 * @return Bitboard of attacked squares.
		 */
		inline bitboard knight(int sq) {
			return knight_attacks[sq];
		}

		/**
		 * Looks up bishop attacks from a source square and occupancy.
		 *
		 * @param sq Source square.
		 * @param occ Board global occupancy.
		 * @return Bitboard of attacked squares.
		 */
		inline bitboard bishop(int sq, bitboard occ) {
			assert(square::is_valid(sq));

			return bishop_attacks[sq][magic_index(occ & magic::bishop_masks[sq], magic::bishop_magics[sq], magic::bishop_bits[sq])];
		}

		/**
		 * Looks up rook attacks from a source square and occupancy.
		 *
		 * @param sq Source square.
		 * @param occ Board global occupancy.
		 * @return Bitboard of attacked squares.
		 */
		inline bitboard rook(int sq, bitboard occ) {
			assert(square::is_valid(sq));

			return rook_attacks[sq][magic_index(occ & magic::rook_masks[sq], magic::rook_magics[sq], magic::rook_bits[sq])];
		}

		/**
		 * Looks up queen attacks from a source square and occupancy.
		 *
		 * @param sq Source square.
		 * @param occ Board global occupancy.
		 * @return Bitboard of attacked squares.
		 */
		inline bitboard queen(int sq, bitboard occ) {
			return rook(sq, occ) | bishop(sq, occ);
		}
	}
}
