/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

namespace neocortex {
	namespace color {
		/* Piece colors */
		constexpr int WHITE = 0;
		constexpr int BLACK = 1;

		/**
		 * Tests if a color is null.
		 *
		 * @param col Input color.
		 * @return true if input color is null.
		 */
		inline bool is_null(int col) {
			return col < 0;
		}

		/**
		 * Gets a null color.
		 *
		 * @return null color.
		 */
		inline int null() {
			return -1;
		}

		/**
		 * Converts a UCI color char into a color.
		 *
		 * @param uci Input character.
		 * @return Parsed color, or null if invalid.
		 */
		int from_uci(char uci);

		/**
		 * Converts a color into a UCI color char.
		 *
		 * @param col Input color.
		 * @return UCI color char or '?' if invalid.
		 */
		char to_uci(int type);
	}
}