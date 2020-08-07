/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

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
		 * Constructs a piece from a color and type.
		 *
		 * @param color Input color.
		 * @param type Input type.
		 *
		 * @return Piece value.
		 */
		int make_piece(int color, int type);

		/**
		 * Gets the color of a piece.
		 *
		 * @param piece Input piece.
		 * @return Piece color.
		 */
		int color(int piece);

		/**
		 * Gets the type of a piece.
		 *
		 * @param piece Input piece.
		 * @return Piece type.
		 */
		int type(int piece);

		/**
		 * Tests if a piece is not null.
		 *
		 * @param piece Input piece.
		 * @return true iff piece is a valid piece.
		 */
		bool is_valid(int piece);

		/**
		 * Tests if a value represents a type.
		 *
		 * @param piece Input type.
		 * @return true iff piece is a valid type.
		 */
		bool is_type(int type);

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
