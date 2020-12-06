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
			int in_check = 0;

			bool was_en_passant = false;
			bool was_castle = false;

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
		 * @return true if color to move is in check, false otherwise.
		 */
		bool check();

		/**
		 * Test if the last move made was a capture.
		 *
		 * @return true if a capture was made, false otherwise.
		 */
		bool capture();

		/**
		 * Test if the last move made was an en passant capture.
		 *
		 * @return true if last move was en passant, false otherwise.
		 */
		bool en_passant();

		/**
		 * Test if the last move made was a promotion.
		 *
		 * @return true if last move was a promotion, false otherwise.
		 */
		bool promotion();

		/**
		 * Test if the last move made was a castle.
		 *
		 * @return true if last move was a castle, false otherwise.
		 */
		bool castle();

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
		 * Gets evasion moves for a position.
		 *
		 * @param dst Buffer to fill with moves. Must be MAX_PL_MOVES size.
		 * @return Number of moves generated.
		 */
		int pseudolegal_moves_evasions(Move* dst);

		/**
		 * Performs move ordering on a list of pseudolegal moves.
		 *
		 * @param moves Move list pointer.
		 * @param num_moves Number of moves in the list.
		 * @param pv_move PV move if available.
		 */
		void order_moves(Move* moves, int num_moves, Move pv_move = Move::null);

		/**
		 * Gets the pseudolegal moves for the position (quiescence search variant)
		 *
		 * @param dst Buffer to fill with moves. Must be MAX_PL_MOVES size.
		 * @return Number of moves generated.
		 */
		int pseudolegal_moves_quiescence(Move* dst);

		/**
		 * Performs move ordering on a list of pseudolegal moves (quiescence search variant)
		 *
		 * @param moves Move list pointer.
		 * @param num_moves Number of moves in the list.
		 * @param pv_move PV move if available.
		 * @return New size of move list.
		 */
		int order_moves_quiescence(Move* moves, int num_moves, Move pv_move = Move::null);

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
		 * @param col Attacking color.
		 */
		int see(int sq, int col);

		/**
		 * Get a printable debug dump of the position.
		 * 
		 * @return Printable debug string.
		 */
		std::string dump();
	private:
		Board board;
		std::vector<State> ply;
		int color_to_move;

		bool test_check(int col) {
			return (board.attacks_on(bb::getlsb(board.get_piece_occ(piece::KING) & board.get_color_occ(col))) & board.get_color_occ(!col)) != 0;
		}
	};

	inline int Position::get_color_to_move() {
		return color_to_move;
	}

	inline bool Position::capture() {
		return ply.back().captured_piece != piece::null;
	}

	inline bool Position::check() {
		return ply.back().in_check;
	}

	inline bool Position::en_passant() {
		return ply.back().was_en_passant;
	}

	inline bool Position::castle() {
		return ply.back().was_castle;
	}

	inline bool Position::promotion() {
		return piece::is_type(ply.back().last_move.ptype());
	}
}
