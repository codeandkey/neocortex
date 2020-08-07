/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <string>

namespace neocortex {
	namespace square {
		constexpr int null = -1;

		/**
		 * Gets a square from board coordinates.
		 *
		 * @param rank Input rank (0-7 inclusive).
		 * @param file Input file (0-7 inclusive).
		 */
		int at(int rank, int file);

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
		int rank(int sq);

		/**
		 * Gets a square's file.
		 *
		 * @param sq Input square.
		 * @return Square file index.
		 */
		int file(int sq);

		/**
		 * Converts a square to a FEN/UCI string.
		 *
		 * @param sq Input square.
		 * @return FEN string.
		 */
		std::string to_uci(int sq);

		/**
		 * Tests if a square is non-null.
		 *
		 * @param sq Input square.
		 * @return true if square is not null, false otherwise.
		 */
		bool is_valid(int sq);
	}
}
