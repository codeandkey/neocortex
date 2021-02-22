/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <cassert>

namespace neocortex {
	namespace piece {
		/**
		 * Tests if a piece is null.
		 *
		 * @param piece Input piece.
		 * @return true if piece is null, false otherwise.
		 */
		inline bool is_null(int piece) {
			return piece < 0;
		}

		/**
		 * Gets a null piece.
		 *
		 * @return null piece.
		 */
		inline int null() {
			return -1;
		}

		/**
		 * Constructs a piece from a color and type.
		 *
		 * @param color Input color.
		 * @param type Input type.
		 *
		 * @return Piece value.
		 */
		inline int make(int color, int type) {
			return (type << 1) | color;
		}

		/**
		 * Gets the color of a piece.
		 *
		 * @param piece Input piece.
		 * @return Piece color.
		 */
		inline int color(int piece) {
			return piece & 1;
		}

		/**
		 * Gets the type of a piece.
		 *
		 * @param piece Input piece.
		 * @return Piece type.
		 */
		inline int type(int piece) {
			return piece >> 1;
		}

		/**
		 * Tests if a value represents a type.
		 *
		 * @param piece Input type.
		 * @return true iff piece is a valid type.
		 */
		inline bool is_type(int type) {
			return type >= 0 && type < 6;
		}

		/**
		 * Gets a UCI character for a piece.
		 * 
		 * @param piece Input piece.
		 * @return UCI piece character, or '?' if invalid.
		 */
		char get_uci(int piece);

		/**
		 * Parses a UCI character into a piece.
		 *
		 * @param uci Input character.
		 * @return Encoded piece.
		 */
		int from_uci(char uci);
	}
}
