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

		extern const int MATERIAL_MG_LOOKUP[12];
		extern const int MATERIAL_EG_LOOKUP[12];

		extern const int MATERIAL_MG_MAX;
		extern const int MATERIAL_EG_MAX;

		constexpr int CONTEMPT = 0;

		extern const int PHASE_VALS[6];
		extern const int PHASE_TOTAL;

		constexpr int TEMPO_BONUS = 4;
		constexpr int CENTER_CONTROL = 1;
		constexpr int KING_SAFETY = 7;
		constexpr int PASSED_PAWNS = 11;
		constexpr int ADV_PASSEDPAWN = 8;
		constexpr int DEVELOPMENT = 15;
		constexpr int FIRST_RANK_KING_MG = 7;
		constexpr int PAWNS_PROT_KING_MG = 2;
		constexpr int EDGE_KNIGHTS = -3;
		constexpr int ISOLATED_PAWNS = -8;
		constexpr int DOUBLED_PAWNS = -6;

		constexpr int OPEN_FILE_ROOK = 8;
		constexpr int OPEN_FILE_QUEEN = 4;

		constexpr int ORDER_PV_MOVE = 40000;

		extern const int GUARD_VALUES[12];
	}
}
