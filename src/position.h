#pragma once

#include "board.h"
#include "move.h"
#include "piece.h"
#include "square.h"

#include <iostream>
#include <string>
#include <vector>

namespace pine {
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
			zobrist::key key;
		};

		Position();
		Position(std::string fen);

		std::string to_fen();

		Board& get_board();
		int get_color_to_move();

		bool make_move(Move move);
		void unmake_move(Move move);

		bitboard en_passant_mask(); /* gets a mask of valid en passant capture targets, or 0 if none */

		/* the following do a full computation, do not used cached values */
		bitboard attacking_squares(); /* squares attacked by color to move */
		bitboard attacked_squares(); /* squares attacked by not color to move */
		bitboard squares_attacked_by(int color); /* squares attacked by color */
		bool square_is_attacked(int sq, int color); /* early-exit test for square attacked by color*/
		void get_attackers_defenders(int sq, int& white, int& black);

		/* get cached attack boards */
		bitboard get_current_attacks(int color);

		zobrist::key get_tt_key();

		bool check();
		bool quiet();
		int castle_rights();
		int num_repetitions();
		std::string game_string();
	private:
		Board board;
		std::vector<State> ply;
		int color_to_move;
	};
}