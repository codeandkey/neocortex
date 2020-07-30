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

namespace pine {
	class Board {
	public:
		Board();
		Board(std::string uci_data);

		static Board standard();

		void place(int sq, int piece);
		int remove(int sq);

		std::string to_uci();
		std::string to_pretty();

		bitboard get_global_occ();
		bitboard get_color_occ(int col);
		bitboard get_piece_occ(int ptype);

		int get_piece(int sq);

		zobrist::key get_tt_key();
	private:
		bitboard global_occ, color_occ[2], piece_occ[6];
		int state[64];
		zobrist::key key;
	};
}