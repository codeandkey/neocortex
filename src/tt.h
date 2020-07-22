#pragma once

#include "move.h"
#include "zobrist.h"

namespace pine {
	namespace tt {
		struct entry {
			zobrist::key key;
			int pv_move;
		};

		constexpr int DEFAULT_BITS = 16;

		void init(int bits = DEFAULT_BITS);
		entry* lookup(zobrist::key key);
	}
}