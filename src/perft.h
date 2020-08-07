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

			/**
			 * Converts the result to a table row for the output.
			 *
			 * @param for_depth Depth this result was executed for.
			 * @return Table row string.
			 */
			std::string to_row(int for_depth);

			/**
			 * Gets a header for the perft result table.
			 *
			 * @return Table header string.
			 */
			static std::string header();
		};

		/**
		 * Runs perft at a single depth.
		 *
		 * @param p Root position.
		 * @param depth Depth to search.
		 */
		results run(Position& p, int depth);

		/**
		 * Runs perft from depth 1 up to the input depth, and outputs
		 * the result in a table form to a stream.
		 *
		 * @param p Root position.
		 * @param depth Maximum depth to run perft.
		 * @param out Output stream.
		 */
		void start(Position& p, int depth, std::ostream& out);
	}
}
