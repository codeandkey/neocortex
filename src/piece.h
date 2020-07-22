#pragma once

namespace pine {
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

		int make_piece(int color, int type);
		int color(int piece);
		int type(int piece);

		bool is_valid(int piece);
		bool is_type(int type);

		char get_uci(int piece);
		int from_uci(char uci);

		int color_from_uci(char uci); /* parses 'w' or 'b' */
		char color_to_uci(int col);

		int type_from_uci(char uci);
		char type_to_uci(int type);
	}
}