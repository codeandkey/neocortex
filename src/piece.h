/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <cassert>

namespace neocortex {
	namespace piece {
		/* Piece types */
		constexpr int PAWN = 0;
		constexpr int BISHOP = 1;
		constexpr int KNIGHT = 2;
		constexpr int ROOK = 3;
		constexpr int QUEEN = 4;
		constexpr int KING = 5;

		/* Piece colors */
		constexpr int WHITE = 0;
		constexpr int BLACK = 1;

		constexpr int null = -1;

		/**
		 * Tests if a piece is not null.
		 *
		 * @param piece Input piece.
		 * @return true iff piece is a valid piece.
		 */
		inline bool is_valid(int piece) {
			return piece >= 0 && piece < 12;
		}

		/**
		 * Constructs a piece from a color and type.
		 *
		 * @param color Input color.
		 * @param type Input type.
		 *
		 * @return Piece value.
		 */
		inline int make_piece(int color, int type) {
			assert(color == WHITE || color == BLACK);
			return (type << 1) | color;
		}

		/**
		 * Gets the color of a piece.
		 *
		 * @param piece Input piece.
		 * @return Piece color.
		 */
		inline int color(int piece) {
			assert(is_valid(piece));
			return piece & 1;
		}

		/**
		 * Gets the type of a piece.
		 *
		 * @param piece Input piece.
		 * @return Piece type.
		 */
		inline int type(int piece) {
			assert(is_valid(piece));
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
		 * Gets a UCI/FEN character for a piece.
		 * 
		 * @param piece Input piece.
		 * @return Piece character in FEN format.
		 */
		char get_uci(int piece);

		/**
		 * Parses a UCI/FEN character into a piece.
		 *
		 * @param uci Input character.
		 * @return Encoded piece.
		 */
		int from_uci(char uci);

		/**
		 * Parses 'w' or 'b' into WHITE and BLACK respectively.
		 *
		 * @param uci Input character.
		 * @return Parsed color.
		 */
		int color_from_uci(char uci);

		/**
		 * Converts a color into 'w' or 'b'.
		 *
		 * @param col Input color.
		 * @return FEN color character ('w' or 'b').
		 */
		char color_to_uci(int col);

		/**
		 * Converts a FEN/UCI piece type char into a type.
		 *
		 * @param uci Input character.
		 * @return Parsed type.
		 */
		int type_from_uci(char uci);

		/**
		 * Converts a piece type into a FEN/UCI piece type char.
		 *
		 * @param type Input type.
		 * @return FEN type character.
		 */
		char type_to_uci(int type);
	}
}
