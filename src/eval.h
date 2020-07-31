/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "position.h"

#include <climits>
#include <string>

namespace pine {
	namespace score {
		bool is_mate(int value);
		int parent(int value);

		std::string to_string(int value);
		std::string to_uci(int value);

		static constexpr int CHECKMATE = 4000000;
		static constexpr int CHECKMATED = -CHECKMATE;
		static constexpr int INCOMPLETE = INT_MIN;
		static constexpr int MAX = INT_MAX; /* should be used for AB bounds, not valid scores */
		static constexpr int MIN = INT_MIN;
		static constexpr int MATE_THRESHOLD = 32;
	}

	class Eval {
	public:
	public:
		Eval(Position& pos);

		int to_score();
		std::string to_table();
	private:
		Position& pos;
		int phase;

		int material_mg[2];
		int material_eg[2];
		int attackbonus[2];
		int mobility_mg[2], mobility_eg[2];
		int center_control[2];
		int king_safety[2];
		int blocking_pawns[2];
		int passed_pawns[2];
		int adv_pawn_mg[2], adv_pawn_eg[2];
		int adv_passedpawn_mg[2], adv_passedpawn_eg[2];
		int king_adv_mg[2], king_adv_eg[2];
	};
}
