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
			bitboard attacks[2];
			zobrist::Key key;
		};

		/**
		 * Constructs a standard start position.
		 */
		Position();

		/**
		 * Constructs a position for a FEN.
		 *
		 * @param fen Input FEN.
		 */
		Position(std::string fen);

		/**
		 * Converts a position to a FEN.
		 *
		 * @retrun FEN string.
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
		 * Computes all squares attacked by the color to move.
		 *
		 * @return Attacked squares.
		 */
		bitboard attacking_squares(); /* squares attacked by color to move */

		/**
		 * Computes all squares attacked by the color not to move.
		 *
		 * @return Attacked squares.
		 */
		bitboard attacked_squares(); /* squares attacked by not color to move */

		/**
		 * Computes all squares attacked by a specific color.
		 *
		 * @param color Attacking color.
		 * @return Attacked squares.
		 */
		bitboard squares_attacked_by(int color); /* squares attacked by color */

		/**
		 * Faster test to determine if a square is attacked by a color.
		 *
		 * @param sq Target square.
		 * @param color Attacking color.
		 * @return true if the square is attacked by the color, false otherwise.
		 */
		bool square_is_attacked(int sq, int color); /* early-exit test for square attacked by color*/

		/**
		 * Counts the attackers from each color on a certain square.
		 *
		 * @param sq Target square.
		 * @param white White attackers output.
		 * @param black Black attackers output.
		 */
		void get_attackers_defenders(int sq, int& white, int& black);

		/**
		 * Gets the current attack bitboard for a color.
		 * Pulled from the ply and does not require computation.
		 * This should be used by all other systems.
		 *
		 * @param color Attacking color.
		 * @return Attack bitboard.
		 */
		bitboard get_current_attacks(int color);

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
	private:
		Board board;
		std::vector<State> ply;
		int color_to_move;
	};

	inline int Position::get_color_to_move() {
		return color_to_move;
	}

	inline bitboard Position::attacking_squares() {
		return ply.back().attacks[color_to_move];
	}

	inline bitboard Position::attacked_squares() {
		return ply.back().attacks[!color_to_move];
	}
}
