/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "square.h"
#include "platform.h"

#ifdef NEOCORTEX_WIN32
#include <intrin.h>
#endif

#include <cassert>
#include <cstdint>
#include <string>

namespace neocortex {
	typedef uint64_t bitboard;

	/* Rank masks */
	constexpr bitboard RANK_1 = 0x00000000000000FFULL;
	constexpr bitboard RANK_2 = RANK_1 << 8;
	constexpr bitboard RANK_3 = RANK_1 << 16;
	constexpr bitboard RANK_4 = RANK_1 << 24;
	constexpr bitboard RANK_5 = RANK_1 << 32;
	constexpr bitboard RANK_6 = RANK_1 << 40;
	constexpr bitboard RANK_7 = RANK_1 << 48;
	constexpr bitboard RANK_8 = RANK_1 << 56;

	/* File masks */
	constexpr bitboard FILE_A = 0x0101010101010101ULL;
	constexpr bitboard FILE_B = FILE_A << 1;
	constexpr bitboard FILE_C = FILE_A << 2;
	constexpr bitboard FILE_D = FILE_A << 3;
	constexpr bitboard FILE_E = FILE_A << 4;
	constexpr bitboard FILE_F = FILE_A << 5;
	constexpr bitboard FILE_G = FILE_A << 6;
	constexpr bitboard FILE_H = FILE_A << 7;

	constexpr bitboard BORDER = (RANK_1 | RANK_8 | FILE_A | FILE_H);

	/* Directional square offsets */
	constexpr int EAST = 1;
	constexpr int WEST = -1;
	constexpr int NORTH = 8;
	constexpr int SOUTH = -8;
	constexpr int NORTHWEST = 7;
	constexpr int NORTHEAST = 9;
	constexpr int SOUTHEAST = -7;
	constexpr int SOUTHWEST = -9;

	namespace bb {
		extern bitboard BETWEEN[64][64];
		extern bitboard NEIGHBOR_FILES[64];

		/**
		 * Initializes static bitboard constants.
		 */
		void init();

		/**
		 * Gets a mask of all squares between two squares.
		 * 
		 * @param a First square.
		 * @param b Second square.
		 * @return Mask of squares between a and b.
		 */
		inline bitboard between(int a, int b) {
			return BETWEEN[a][b];
		}

		/**
		 * Gets a mask of neighboring files to a square.
		 * @param sq Square.
		 * @return Files adjacent to sq.
		 */
		inline bitboard neighbor_files(int sq) {
			return NEIGHBOR_FILES[sq];
		}

		/**
		 * Builds a readable string from a bitboard.
		 * Output will consist of 8 rows of 8 characters, with '.' and 'X' indicating 0 and 1 bits respectively.
		 *
		 * @param b Input bitboard.
		 * @return Printable bitboard string.
		 */
		std::string to_string(bitboard b);

		/**
		 * Locates the position of the least significant '1' bit in a bitboard.
		 * Equivalent to locating the "next" square in a set.
		 *
		 * @param b Input bitboard. Must have at least one square set.
		 * @return int Least significant square set in bitboard.
		 */
		inline int getlsb(bitboard b) {
			assert(b);

#ifdef NEOCORTEX_WIN32
			unsigned long pos;
			_BitScanForward64(&pos, b);

			return (int) pos;
#elif defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
			return (int) __builtin_ctzll(b);
#endif
		}

		/**
		 * Equivalent to getlsb() except the returned square is then removed from the bitboard.
		 *
		 * @param b Input bitboard. Modified in place!
		 * @return Least significant square set in bitboard.
		 */
		inline int poplsb(bitboard& b) {
			int lsb = getlsb(b);
			b ^= 1ULL << lsb;
			return lsb;
		}

		/**
		 * Bitwise shift on a bitboard, with support for negative shifts.
		 *
		 * @param b Input bitboard.
		 * @param dir Shift direction.
		 *
		 * @return Shifted bitboard.
		 */
		inline bitboard shift(bitboard b, int dir) {
			return (dir > 0) ? (b << dir) : (b >> -dir);
		}

		/**
		 * Generates a bitboard with a single square set.
		 *
		 * @param sq Input square.
		 * @return Bitboard with 'sq' set to 1.
		 */
		inline bitboard mask(int sq) {
			assert(square::is_valid(sq));

			return ((bitboard) 1) << sq;
		}

		/**
		 * Returns the count of set bits in a bitboard.
		 *
		 * @param b Input bitboard.
		 * @return Number of '1' bits in input.
		 */
		inline int popcount(bitboard b) {
#ifdef NEOCORTEX_WIN32
			return (int) __popcnt64(b);
#elif defined NEOCORTEX_LINUX || defined NEOCORTEX_OSX
			return (int) __builtin_popcountll(b);
#endif
		}

		/**
		 * Gets a mask for a rank by index.
		 *
		 * @param r Input rank. Must be between 0 and 7 inclusive.
		 * @return Mask for the (r + 1)th rank.
		 */
		inline bitboard rank(int r) {
			assert(r >= 0 && r < 8);
			return RANK_1 << (8 * r);
		}

		/**
		 * Gets a mask for a file by index.
		 *
		 * @param f Input file. Must be between 0 and 7 inclusive.
		 * @return Mask for the (f + 1)th file.
		 */
		inline bitboard file(int f) {
			assert(f >= 0 && f < 8);
			return FILE_A << f;
		}
	}
}
