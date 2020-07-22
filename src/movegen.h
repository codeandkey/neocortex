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

		class Generator {
		public:
			Generator(Position& position);

			std::list<Move> next_batch();
			std::list<Move> generate_stage(int stage); /* generate moves in a specific stage */
			std::list<Move> generate(); /* empty => no more moves */
			Move next_move(); /* returns null move if no more in batch, then generate() must be called again */

			std::list<Move> pawn_jumps(); /* pawn doublejumps */
			std::list<Move> pawn_advances(); /* pawn doublejumps */
			std::list<Move> pawn_promotions(); /* pawn promotions */
			std::list<Move> captures(int attacker, int victim); /* all captures */
			std::list<Move> quiets(int type);
			std::list<Move> castles();

			std::list<std::list<Move>> generate_perft(); /* generate all pseudolegal moves for perft */
		private:
			int stage;

			Position& position;

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