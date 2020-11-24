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

		constexpr int TEMPO_BONUS = 10;
		constexpr int CENTER_CONTROL = 25;
		constexpr int KING_SAFETY = 10;
		constexpr int PASSED_PAWNS = 30;
		constexpr int ADV_PASSEDPAWN = 15;
		constexpr int KING_ADV_MG = -35;
		constexpr int KING_ADV_EG = 10;

		constexpr int ORDER_PV_MOVE = 40000;

		extern const int GUARD_VALUES[12];
	}
}
