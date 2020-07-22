#pragma once

#include "bitboard.h"

namespace pine {
	namespace attacks {
		void init();

		bitboard king(int sq);
		bitboard knight(int sq);
		bitboard bishop(int sq, bitboard occ);
		bitboard rook(int sq, bitboard occ);
		bitboard queen(int sq, bitboard occ);
	}
}