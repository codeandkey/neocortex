/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "bitboard.h"
#include "piece.h"
#include "square.h"
#include "zobrist.h"

namespace neocortex {
	class Board {
	public:
		/**
		 * Constructs an empty board.
		 * The board will have no pieces placed.
		 */
		Board();

		/**
		 * Constructs a board from a FEN string.
		 * The format should match the first field of a valid FEN.
		 *
		 * @param fen_data Input FEN data.
		 */
		Board(std::string fen_data);

		/**
		 * Constructs a board matching the standard start position.
		 */
		static Board standard();

		/**
		 * Places a piece on the board.
		 *
		 * @param sq Destination square.
		 * @param piece Piece to place.
		 */
		void place(int sq, int piece);

		/**
		 * Removes a piece from the board.
		 *
		 * @param sq Square to remove from.
		 * @return The removed piece.
		 */
		int remove(int sq);

		/**
		 * Converts the board state to a FEN field.
		 *
		 * @return FEN field for board state.
		 */
		std::string to_uci();

		/**
		 * Converts the board to a human-readable string.
		 * The string has appropriate newlines and can be printed.
		 *
		 * @return Human-readable board string.
		 */
		std::string to_pretty();

		/**
		 * Gets the global occupancy bitboard.
		 *
		 * @return Occupancy bitboard.
		 */
		bitboard get_global_occ();

		/**
		 * Gets a color occupancy bitboard.
		 *
		 * @param col Color to retrieve.
		 * @return Occupancy bitboard.
		 */
		bitboard get_color_occ(int col);

		/**
		 * Gets a piece type occupancy bitboard.
		 *
		 * @param ptype Piece type to retrieve.
		 * @return Occupancy bitboard.
		 */
		bitboard get_piece_occ(int ptype);

		/**
		 * Gets the piece on a square, or null if there is none.
		 *
		 * @param sq Square to retrieve.
		 * @return Piece on square, or null piece if none present.
		 */
		int get_piece(int sq);

		/**
		 * Gets the occupancy of a square.
		 *
		 * @param sq Square to retrieve.
		 * @return true if the square is occupied.
		 */
		bool is_occ(int sq);

		/**
		 * Gets the zobrist key for the board state.
		 *
		 * @return Current zobrist key.
		 */
		zobrist::Key get_tt_key();
	private:
		bitboard global_occ, color_occ[2], piece_occ[6];
		int state[64];
		zobrist::Key key;
	};

	inline int Board::get_piece(int sq) {
		return state[sq];
	}

	inline bitboard Board::get_global_occ() {
		return global_occ;
	}

	inline bitboard Board::get_color_occ(int col) {
		return color_occ[col];
	}

	inline bitboard Board::get_piece_occ(int t) {
		return piece_occ[t];
	}

	inline bool Board::is_occ(int sq) {
		return piece::is_valid(state[sq]);
	}

	inline zobrist::Key Board::get_tt_key() {
		return key;
	}
}
