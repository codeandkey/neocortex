/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "position.h"

namespace neocortex {
	namespace perft {
		struct results {
			unsigned long nodes;
			unsigned long checks;
			unsigned long captures;
			unsigned long castles;
			unsigned long nps;

			double totaltime;

			std::string to_row(int for_depth);
			static std::string header();
		};

		results run(Position& p, int depth);
		void start(Position& p, int depth, std::ostream& out);
	}
}
