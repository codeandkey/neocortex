/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "bitboard.h"

using namespace neocortex;

bitboard bb::BETWEEN[64][64];

void bb::init() {
	/* zero lookup buf */
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j) {
			BETWEEN[i][j] = 0ULL;
		}
	}

	/* construct lookup */
	for (int src = 0; src < 64; ++src) {
		/* west */
		if (square::file(src) > 1) {
			int t = src - 2;

			do {
				for (int j = src - 1; j != t; --j) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::file(t--) != 0);
		}

		/* east */
		if (square::file(src) < 6) {
			int t = src + 2;

			do {
				for (int j = src + 1; j != t; ++j) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::file(t++) != 7);
		}

		/* north */
		if (square::rank(src) < 6) {
			int t = src + NORTH;

			do {
				t += NORTH;

				for (int j = src + NORTH; j != t; j += NORTH) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::rank(t) != 7);
		}

		/* south */
		if (square::rank(src) > 1) {
			int t = src + SOUTH;

			do {
				t += SOUTH;

				for (int j = src + SOUTH; j != t; j += SOUTH) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::rank(t) != 0);
		}
		
		/* northeast */
		if (square::rank(src) < 6 && square::file(src) < 6) {
			int t = src + NORTHEAST;

			do {
				t += NORTHEAST;

				for (int j = src + NORTHEAST; j != t; j += NORTHEAST) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::rank(t) != 7 && square::file(t) != 7);
		}

		/* northwest */
		if (square::rank(src) < 6 && square::file(src) > 1) {
			int t = src + NORTHWEST;

			do {
				t += NORTHWEST;

				for (int j = src + NORTHWEST; j != t; j += NORTHWEST) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::rank(t) != 7 && square::file(t) != 0);
		}

		/* southeast */
		if (square::rank(src) > 1 && square::file(src) < 6) {
			int t = src + SOUTHEAST;

			do {
				t += SOUTHEAST;

				for (int j = src + SOUTHEAST; j != t; j += SOUTHEAST) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::rank(t) != 0 && square::file(t) != 7);
		}

		/* southwest */
		if (square::rank(src) > 1 && square::file(src) > 1) {
			int t = src + SOUTHWEST;

			do {
				t += SOUTHWEST;

				for (int j = src + SOUTHWEST; j != t; j += SOUTHWEST) {
					BETWEEN[src][t] |= 1ULL << j;
				}
			} while (square::rank(t) != 0 && square::file(t) != 0);
		}
	}
}

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
