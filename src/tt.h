#pragma once

#include "move.h"
#include "zobrist.h"
#include "eval.h"

namespace pine {
	namespace tt {
		struct entry {
			zobrist::key key = 0;
			Move pv_move = Move::null;
			int type = EXACT;
			int value = 0;
			int depth = 0;

			static constexpr int LOWERBOUND = 0;
			static constexpr int UPPERBOUND = 1;
			static constexpr int EXACT = 2;
		};

		constexpr size_t DEFAULT_MIB = 128;

		void init(size_t mib = DEFAULT_MIB);
		entry* lookup(zobrist::key key);
	}
}