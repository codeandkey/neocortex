/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "bitboard.h"

using namespace neocortex;

std::string bb::to_string(bitboard inp) {
	std::string output;

	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f < 8; ++f) {
			if (bb::mask(square::at(r, f)) & inp) {
				output += '1';
			} else {
				output += '.';
			}
		}

		output += '\n';
	}

	return output;
}
