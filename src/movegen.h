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

namespace neocortex {
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
			/**
			 * Constructs a new move generator.
			 *
			 * @param position Position to generate for.
			 * @param pv_move PV move to emit first, if present.
			 * @param mode Move generation mode.
			 */
			Generator(Position& position, Move pv_move = Move::null, int mode = NORMAL);

			/**
			 * Sets the killer moves to generate from.
			 */
			void set_killers(Move* killers);

			/**
			 * Gets the next pseudolegal move.
			 * Returns a null move if there are none left.
			 *
			 * @return Next pseudolegal move.
			 */
			Move next();

			/**
			 * Generates all pseudolegal moves simultaneously without staging.
			 * Used for perft runs.
			 *
			 * @return All pseudolegal moves.
			 */
			std::list<std::list<Move>> generate_perft(); /* generate all pseudolegal moves for perft */
		private:
			/**
			 * Gets the next stage batch of moves. Advances the stage.
			 * Used internally.
			 *
			 * @return Next stage batch.
			 */
			std::list<Move> next_batch();

			/**
			 * Gets the next batch of moves. Always returns at least one move,
			 * or an empty list if the movegen is completed.
			 *
			 * @return Next move batch.
			 */
			std::list<Move> generate();

			/**
			 * Generates pawn double jump moves for the position.
			 *
			 * @return Pseudolegal moves.
			 */
			std::list<Move> pawn_jumps();

			/**
			 * Generates nonpromoting pawn advances for the position.
			 *
			 * @return Pseudolegal moves.
			 */
			std::list<Move> pawn_advances();

			/**
			 * Generates promoting pawn moves for the position.
			 *
			 * @return Pseudolegal moves.
			 */
			std::list<Move> pawn_promotions(); /* pawn promotions */

			/**
			 * Generates captures for the position.
			 *
			 * @param attacker Attacking piece type.
			 * @param victim Victim piece type.
			 *
			 * @return Pseudolegal moves.
			 */
			std::list<Move> captures(int attacker, int victim); /* all captures */

			/**
			 * Generates quiet moves for the position.
			 *
			 * @return Pseudolegal moves.
			 */
			std::list<Move> quiets(int type);

			/**
			 * Generates castles moves for the position.
			 *
			 * @return Pseudolegal moves.
			 */
			std::list<Move> castles();

			int stage, mode;
			Move pv_move, killers[2];

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
