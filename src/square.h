/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <cassert>
#include <string>

namespace neocortex {
	namespace square {
		// Square constants
		constexpr int A1 = 0;
		constexpr int B1 = 1;
		constexpr int C1 = 2;
		constexpr int D1 = 3;
		constexpr int E1 = 4;
		constexpr int F1 = 5;
		constexpr int G1 = 6;
		constexpr int H1 = 7;
		constexpr int A2 = 8;
		constexpr int B2 = 9;
		constexpr int C2 = 10;
		constexpr int D2 = 11;
		constexpr int E2 = 12;
		constexpr int F2 = 13;
		constexpr int G2 = 14;
		constexpr int H2 = 15;
		constexpr int A3 = 16;
		constexpr int B3 = 17;
		constexpr int C3 = 18;
		constexpr int D3 = 19;
		constexpr int E3 = 20;
		constexpr int F3 = 21;
		constexpr int G3 = 22;
		constexpr int H3 = 23;
		constexpr int A4 = 24;
		constexpr int B4 = 25;
		constexpr int C4 = 26;
		constexpr int D4 = 27;
		constexpr int E4 = 28;
		constexpr int F4 = 29;
		constexpr int G4 = 30;
		constexpr int H4 = 31;
		constexpr int A5 = 32;
		constexpr int B5 = 33;
		constexpr int C5 = 34;
		constexpr int D5 = 35;
		constexpr int E5 = 36;
		constexpr int F5 = 37;
		constexpr int G5 = 38;
		constexpr int H5 = 39;
		constexpr int A6 = 40;
		constexpr int B6 = 41;
		constexpr int C6 = 42;
		constexpr int D6 = 43;
		constexpr int E6 = 44;
		constexpr int F6 = 45;
		constexpr int G6 = 46;
		constexpr int H6 = 47;
		constexpr int A7 = 48;
		constexpr int B7 = 49;
		constexpr int C7 = 50;
		constexpr int D7 = 51;
		constexpr int E7 = 52;
		constexpr int F7 = 53;
		constexpr int G7 = 54;
		constexpr int H7 = 55;
		constexpr int A8 = 56;
		constexpr int B8 = 57;
		constexpr int C8 = 58;
		constexpr int D8 = 59;
		constexpr int E8 = 60;
		constexpr int F8 = 61;
		constexpr int G8 = 62;
		constexpr int H8 = 63;

		/**
		 * Tests if a square is null.
		 *
		 * @param sq Input square.
		 * @return true if square is null, false otherwise.
		 */
		inline bool is_null(int sq) {
			return sq < 0;
		}

		/**
		 * Tests if a square is valid and not null.
		 *
		 * @param sq Input square.
		 * @return true if square is valid, false otherwise.
		 */
		inline bool is_valid(int sq) {
			return !is_null(sq) && sq < 64;
		}

		/**
		 * Gets a null square.
		 *
		 * @return null square
		 */
		inline int null() {
			return -1;
		}

		/**
		 * Gets a square from board coordinates.
		 *
		 * @param rank Input rank (0-7 inclusive).
		 * @param file Input file (0-7 inclusive).
		 */
		inline int at(int rank, int file) {
			return rank * 8 + file;
		}

		/**
		 * Parses a square from a FEN/UCI string.
		 *
		 * @param Input string.
		 * @return Encoded square.
		 */
		int from_uci(std::string uci);

		/**
		 * Gets a square's rank.
		 *
		 * @param sq Input square.
		 * @return Square rank index.
		 */
		inline int rank(int sq) {
			return sq >> 3;
		}

		/**
		 * Gets a square's file.
		 *
		 * @param sq Input square.
		 * @return Square file index.
		 */
		inline int file(int sq) {
			return sq & 7;
		}

		/**
		 * Converts a square to a FEN/UCI string.
		 *
		 * @param sq Input square.
		 * @return FEN string.
		 */
		std::string to_uci(int sq);
	}
}
