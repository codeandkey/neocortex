/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "board.h"
#include "move.h"
#include "piece.h"
#include "square.h"

#include <iostream>
#include <string>
#include <vector>

namespace neocortex {
	constexpr int CASTLE_WHITE_K = 1;
	constexpr int CASTLE_WHITE_Q = 2;
	constexpr int CASTLE_BLACK_K = 4;
	constexpr int CASTLE_BLACK_Q = 8;

	class Position {
	public:
		struct State {
			Move last_move;
			int en_passant_square;
			int castle_rights;
			int captured_piece;
			int captured_square;
			int halfmove_clock;
			int fullmove_number;
			int attacks[2][64];
			zobrist::Key key;
		};

		/**
		 * Constructs a position for a FEN.
		 *
		 * @param fen Input FEN.
		 */
		Position(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

		/**
		 * Converts a position to a FEN.
		 *
		 * @return FEN string.
		 */
		std::string to_fen();

		/**
		 * Gets a reference to the position's current board.
		 *
		 * @return Board reference.
		 */
		Board& get_board();

		/**
		 * Gets the current color to move.
		 *
		 * @return Color to move.
		 */
		int get_color_to_move();

		/**
		 * Tries to make a move.
		 * 
		 * @param move Pseudolegal move.
		 * @return true if move is legal, false otherwise.
		 */
		bool make_move(Move move);

		/**
		 * Unmakes a move. The move must match the last move made.
		 *
		 * @param move Last move.
		 */
		void unmake_move(Move move);

		/**
		 * Gets a mask of valid en passant squares.
		 *
		 * @return En-passant mask.
		 */
		bitboard en_passant_mask(); /* gets a mask of valid en passant capture targets, or 0 if none */

		/**
		 * Gets the position's zobrist key for TT indexing.
		 *
		 * @return Zobrist key.
		 */
		zobrist::Key get_tt_key();

		/**
		 * Test if the position is a check.
		 *
		 * @return true if the color to move is in check, false otherwise.
		 */
		bool check();

		/**
		 * Test if a position is quiet.
		 * A position is quiet if no capture or check was made.
		 *
		 * @return true if position is quiet, false otherwise.
		 */
		bool quiet();

		/**
		 * Gets the castle rights mask for both colors.
		 *
		 * @return Castle rights mask.
		 */
		int castle_rights();

		/**
		 * Gets the number of times the current position has occurred throughout the game.
		 *
		 * @return Number of instances. Will always be at least 1.
		 */
		int num_repetitions();

		/**
		 * Gets the game's halfmove clock.
		 *
		 * @return Halfmove clock.
		 */
		int halfmove_clock();

		/**
		 * Gets the game as a PGN string.
		 *
		 * @return PGN string.
		 */
		std::string game_string();

		/**
		 * Gets the location of a color's king.
		 *
		 * @param col Target color.
		 * @return King square.
		 */
		inline int king_loc(int col) {
			return bb::getlsb(board.get_piece_occ(piece::KING) & board.get_color_occ(col));
		}
	private:
		Board board;
		std::vector<State> ply;
		int color_to_move;

		/**
		 * Gets attacks from a square.
		 *
		 * @param sq Source square.
		 * @param col Color of piece (output).
		 *
		 * @return Attack bitboard.
		 */
		bitboard get_attacks(int sq, int& col);

		/**
		 * Removes attacks from a set of pieces from the attack map.
		 *
		 * @param sqs Attacking pieces to remove.
		 * @param dst Destination attack map.
		 */
		void remove_attacks(bitboard sqs, int** dst);

		/**
		 * Adds attacks from a set of pieces to the attack map.
		 *
		 * @param sqs Attacking pieces to add.
		 * @param dst Destination attack map.
		 */
		void add_attacks(bitboard sqs, int** dst);
	};

	inline int Position::get_color_to_move() {
		return color_to_move;
	}
}
