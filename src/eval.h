/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "position.h"

#include <climits>
#include <string>

namespace neocortex {
	namespace score {
		/**
		 * Test if a value represents a forced mate for either color.
		 *
		 * @param value Value to test.
		 * @return true iff input is a forced mate.
		 */
		bool is_mate(int value);

		/**
		 * Gets the "parent" for an eval. If the eval is a mate, this function increases the distance-to-mate.
		 * If the eval is not a mate, this function returns the unaltered input.
		 *
		 * @param value Input value.
		 * @return Evaluation parent value.
		 */
		int parent(int value);

		/**
		 * Converts a score into a human readable string.
		 * Scores: "[+/-]<value>"
		 * Mates: "#[+/-]<ply>"
		 *
		 * @param value Input score.
		 * @return Human-readable score string.
		 */
		std::string to_string(int value);

		/**
		 * Converts a score into a UCI-compliant score string.
		 *
		 * @param value Input score.
		 * @return UCI score string.
		 */
		std::string to_uci(int value);

		static constexpr int CHECKMATE = 4000000;
		static constexpr int CHECKMATED = -CHECKMATE;
		static constexpr int INCOMPLETE = INT_MIN;
		static constexpr int MAX = INT_MAX; /* should be used for AB bounds, not valid scores */
		static constexpr int MIN = INT_MIN;
		static constexpr int MATE_THRESHOLD = 32;
	}
}
