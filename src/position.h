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

	constexpr int MAX_PL_MOVES = 100;
	constexpr int SEE_ILLEGAL = -100000;

	class Position {
	public:
		struct State {
			Move last_move = Move::null;
			int en_passant_square = square::null;
			int castle_rights = 0xF;
			int captured_piece = piece::null;
			int captured_square = square::null;
			int halfmove_clock = 0;
			int fullmove_number = 1;
			zobrist::Key key = 0;
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
		 * Gets the position's zobrist key for TT indexing.
		 *
		 * @return Zobrist key.
		 */
		zobrist::Key get_tt_key();

		/**
		 * Test if the position is a check.
		 *
		 * @param col Color to test.
		 * @return true if col is in check, false otherwise.
		 */
		bool check(int col);

		/**
		 * Test if the last move made was a capture.
		 *
		 * @return true if a capture was made, false otherwise.
		 */
		bool capture();

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
		 * Evaluates the current position.
		 *
		 * @param dbg String output pointer for debug information.
		 * @return Evaluation score.
		 */
		int evaluate(std::string* dbg = NULL);

		/**
		 * Gets the pseudolegal moves for the position.
		 * 
		 * @param dst Buffer to fill with moves. Must be MAX_PL_MOVES size.
		 * @return Number of moves generated.
		 */
		int pseudolegal_moves(Move* dst);

		/**
		 * Performs move ordering on a list of pseudolegal moves.
		 *
		 * @param moves Move list pointer.
		 * @param num_moves Number of moves in the list.
		 * @param pv_move PV move if available.
		 */
		void order_moves(Move* moves, int num_moves, Move pv_move = Move::null);

		/**
		 * Performs static exchange evaluation on a capture.
		 * 
		 * @param capture Capturing move.
		 */
		int see_capture(Move capture);

		/**
		 * Performs static exchange evaluation on a square.
		 *
		 * @param sq Destination square.
		 * @param valid_attackers Mask of allowed attackers.
		 */
		int see(int sq, bitboard valid_attackers = ~0);
	private:
		Board board;
		std::vector<State> ply;
		int color_to_move;
	};

	inline int Position::get_color_to_move() {
		return color_to_move;
	}

	inline bool Position::capture() {
		return ply.back().captured_piece != piece::null;
	}

	inline bool Position::check(int col) {
		return board.attacks_on(bb::getlsb(board.get_piece_occ(piece::KING) & board.get_color_occ(col))) & board.get_color_occ(!col);
	}
}
