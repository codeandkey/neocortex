/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "position.h"

#include <list>
#include <queue>

/*
 * Move generation stages:
 *
 * 1. PV move if available
 * 2. Promotion moves
 * 3. Saved killers if available
 * 4. Non-promoting captures in MVV-LVA order
 * 5. Castling moves
 * 6. Quiet moves
 */

namespace pine {
	namespace movegen {
		constexpr int STAGE_PV = 0;
		constexpr int STAGE_PROMOTIONS = 1;
		constexpr int STAGE_KILLERS = 2;
		constexpr int STAGE_CAPTURES = 3;
		constexpr int STAGE_CASTLES = 4;
		constexpr int STAGE_QUIETS = 5;
		constexpr int STAGE_END = 6;

		constexpr int QSTAGE_CAPTURES = 33;
		constexpr int QSTAGE_QUIETS = 34;
		constexpr int QSTAGE_CASTLES = 35;
		constexpr int QSTAGE_PROMOTIONS = 36;
		constexpr int QSTAGE_END = 37;

		constexpr int NORMAL = 0;
		constexpr int QUIESCENCE = 1;

		class Generator {
		public:
			Generator(Position& position, Move pv_move = Move::null, int mode = NORMAL);

			std::list<Move> next_batch();
			std::list<Move> generate();
			Move next();

			std::list<Move> pawn_jumps(); /* pawn doublejumps */
			std::list<Move> pawn_advances(); /* pawn doublejumps */
			std::list<Move> pawn_promotions(); /* pawn promotions */
			std::list<Move> captures(int attacker, int victim); /* all captures */
			std::list<Move> quiets(int type);
			std::list<Move> castles();

			std::list<std::list<Move>> generate_perft(); /* generate all pseudolegal moves for perft */
		private:
			int stage, mode;
			Move pv_move;

			Position& position;
			std::list<Move> batch;

			/* convienience position info */
			bitboard ctm, opp;
			bool white_to_move;
			Board& board;

			int current_attacker;
			int current_victim;
			int current_quiet_index;
		};
	}
}