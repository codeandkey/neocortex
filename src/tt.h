/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "move.h"
#include "zobrist.h"
#include "eval.h"

namespace neocortex {
	namespace tt {
		struct entry {
			zobrist::Key key = 0;
			Move pv_move = Move::null;
			int type = EXACT;
			int value = 0;
			int depth = 0;

			static constexpr int LOWERBOUND = 0;
			static constexpr int UPPERBOUND = 1;
			static constexpr int EXACT = 2;
		};

		constexpr size_t DEFAULT_MIB = 512;

		/**
		 * Initializes the transposition table.
		 * Must be called before any lookups.
		 *
		 * @param mib Table memory to allocate, in MiB.
		 */
		void init(size_t mib = DEFAULT_MIB);

		/**
		 * Looks up an entry in the table.
		 *
		 * @param key Zobrist key.
		 * @return Matching entry for key.
		 */
		entry* lookup(zobrist::Key key);

		/**
		 * Resizes the transposition table.
		 * 
		 * @param mb New size in megabytes.
		 */
		void resize(int mb);

		/**
		 * Acquires a TT mutex lock.
		 *
		 * @param k Zobrist key to lock.
		 */
		void lock(zobrist::Key k);

		/**
		 * Releases a TT mutex lock.
		 *
		 * @param k Zobrist key to unlock.
		 */
		void unlock(zobrist::Key k);
	}
}
