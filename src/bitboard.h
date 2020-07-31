/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "square.h"
#include "platform.h"

#ifdef PINE_WIN32
#include <intrin.h>
#endif

#include <cassert>
#include <cstdint>
#include <string>

namespace pine {
	typedef uint64_t bitboard;

	constexpr bitboard RANK_1 = 0x00000000000000FFULL;
	constexpr bitboard RANK_2 = RANK_1 << 8;
	constexpr bitboard RANK_3 = RANK_1 << 16;
	constexpr bitboard RANK_4 = RANK_1 << 24;
	constexpr bitboard RANK_5 = RANK_1 << 32;
	constexpr bitboard RANK_6 = RANK_1 << 40;
	constexpr bitboard RANK_7 = RANK_1 << 48;
	constexpr bitboard RANK_8 = RANK_1 << 56;

	constexpr bitboard FILE_A = 0x0101010101010101ULL;
	constexpr bitboard FILE_B = FILE_A << 1;
	constexpr bitboard FILE_C = FILE_A << 2;
	constexpr bitboard FILE_D = FILE_A << 3;
	constexpr bitboard FILE_E = FILE_A << 4;
	constexpr bitboard FILE_F = FILE_A << 5;
	constexpr bitboard FILE_G = FILE_A << 6;
	constexpr bitboard FILE_H = FILE_A << 7;

	constexpr bitboard BORDER = (RANK_1 | RANK_8 | FILE_A | FILE_H);

	constexpr int EAST = 1;
	constexpr int WEST = -1;
	constexpr int NORTH = 8;
	constexpr int SOUTH = -8;
	constexpr int NORTHWEST = 7;
	constexpr int NORTHEAST = 9;
	constexpr int SOUTHEAST = -7;
	constexpr int SOUTHWEST = -9;

	namespace bb {
		std::string to_string(bitboard b);

		inline int getlsb(bitboard b) {
			assert(b);

#ifdef PINE_WIN32
			unsigned long pos;
			_BitScanForward64(&pos, b);

			return (int) pos;
#elif defined PINE_LINUX || defined PINE_OSX
			return (int) __builtin_ctzll(b);
#endif
		}

		inline int poplsb(bitboard& b) {
			int lsb = getlsb(b);
			b ^= 1ULL << lsb;
			return lsb;
		}

		inline bitboard shift(bitboard b, int dir) {
			return (dir > 0) ? (b << dir) : (b >> -dir);
		}

		inline bitboard mask(int sq) {
			assert(square::is_valid(sq));

			return ((bitboard) 1) << sq;
		}

		inline int popcount(bitboard b) {
#ifdef PINE_WIN32
			return (int) __popcnt64(b);
#elif defined PINE_LINUX || defined PINE_OSX
			return (int) __builtin_popcountll(b);
#endif
		}

		inline bitboard rank(int r) {
			assert(r >= 0 && r < 8);
			return RANK_1 << (8 * r);
		}

		inline bitboard file(int f) {
			assert(f >= 0 && f < 8);
			return FILE_A << f;
		}
	}
}
