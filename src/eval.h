#pragma once

#include "position.h"

#include <limits>
#include <string>

namespace pine {
	class Score {
	public:
		Score(int value);

		bool is_mate();
		Score parent(); /* increases mate distance if mate */

		std::string to_string();
		std::string to_uci();

		Score operator-();

		static constexpr int CHECKMATE = INT_MAX;
		static constexpr int CHECKMATED = INT_MIN;
		static constexpr int MATE_THRESHOLD = 32;
	private:
		int value;
	};

	class Eval {
	public:
		Eval(Position& pos);

		Score to_score();
		std::string to_table();
	private:
		Position& pos;
		float phase;

		int material_nonpawn_mg;
		int material_mg[2];
		int material_eg[2];
	};
}