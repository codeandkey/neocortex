/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

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
		int getlsb(bitboard b);
		int poplsb(bitboard& b);
		bitboard shift(bitboard b, int dir);
		bitboard mask(int sq);
		int popcount(bitboard b);
		bitboard rank(int r);
		bitboard file(int f);
	}
}