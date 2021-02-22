/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

namespace neocortex {
	namespace type {
		/* Piece types */
		constexpr int PAWN = 0;
		constexpr int BISHOP = 1;
		constexpr int KNIGHT = 2;
		constexpr int ROOK = 3;
		constexpr int QUEEN = 4;
		constexpr int KING = 5;

		/**
		 * Tests if a piece type is null.
		 *
		 * @param piece Input type.
		 * @return true if input type is null.
		 */
		inline bool is_null(int ptype) {
			return ptype < 0;
		}

		/**
		 * Gets a null ptype.
		 *
		 * @return null ptype.
		 */
		inline int null() {
			return -1;
		}

		/**
		 * Converts a UCI piece type char into a ptype.
		 *
		 * @param uci Input character.
		 * @return Parsed type, or null if invalid.
		 */
		int from_uci(char uci);

		/**
		 * Converts a piece type into a UCI piece type char.
		 *
		 * @param type Input type.
		 * @return UCI type character or '?' if invalid.
		 */
		char to_uci(int type);
	}
}