/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

namespace neocortex {
	/**
	 * This namespace contains all constants used in the evaluation function.
	 */

	namespace eval {
		extern const int MATERIAL_MG[6];
		extern const int MATERIAL_EG[6];

		extern const int MATERIAL_MG_MAX;
		extern const int MATERIAL_EG_MAX;

		constexpr int CONTEMPT = 0;
		constexpr int MOBILITY = 2;
		constexpr int ATTACK_BONUS = 2;

		constexpr int MOBILITY_QUEEN = 4;
		constexpr int MOBILITY_KNIGHT = 4;
		constexpr int MOBILITY_BISHOP = 4;
		constexpr int MOBILITY_ROOK_MG = 1;
		constexpr int MOBILITY_ROOK_EG = 4;

		constexpr int TEMPO_BONUS = 10;
		constexpr int CENTER_CONTROL = 25;
		constexpr int KING_SAFETY = 10;
		constexpr int BLOCKING_PAWNS = -50;
		constexpr int PASSED_PAWNS = 30;
		constexpr int ADV_PAWN_MG = 4;
		constexpr int ADV_PASSEDPAWN_MG = 8;
		constexpr int ADV_PAWN_EG = 8;
		constexpr int ADV_PASSEDPAWN_EG = 16;
		constexpr int KING_ADV_MG = -35;
		constexpr int KING_ADV_EG = 10;
	}
}
