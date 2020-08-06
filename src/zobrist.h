/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <cstdint>

namespace neocortex {
	namespace zobrist {
		typedef uint64_t Key;

		void init();

		Key piece(int sq, int piece);
		Key castle(int rights);
		Key en_passant(int file);
		Key black_to_move();
	}
}
