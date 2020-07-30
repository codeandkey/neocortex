/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <cstdint>

namespace pine {
	namespace zobrist {
		typedef uint64_t key;

		void init();

		key piece(int sq, int piece);
		key castle(int rights);
		key en_passant(int file);
		key black_to_move();
	}
}