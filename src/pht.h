/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <mutex>

#include "bitboard.h"
#include "board.h"
#include "zobrist.h"

namespace neocortex {
	namespace pht {
		struct PawnEval {
			zobrist::Key key = 0;
			int backward = 0;
			int straggler = 0;
			int passed = 0;
			int candidates = 0;
			int doubled = 0;
			int isolated = 0;
			int total = 0;
		};

		constexpr int PHT_DEFAULT_SIZE = 128;

		extern PawnEval* pht_buffer;
		extern int pht_buffer_len;
		extern std::mutex pht_mutex;

		/**
		 * Initializes the pawn hash table.
		 */
		void init();

		/**
		 * Resizes the pawn hash table.
		 *
		 * @param mb New PHT size (MB).
		 */
		void resize(unsigned long mb);

		/**
		 * Evaluates a pawn structure.
		 *
		 * @param b Board to evaluate.
		 * @return Pawn evaluation structure.
		 */
		PawnEval evaluate(Board& b);

		/**
		 * Returns a pointer to an entry in the PHT.
		 *
		 * @param pht_key PHT lookup key.
		 * @return Pointer to corresponding PHT entry. Must not be modified while the PHT is unlocked.
		 */
		inline PawnEval* lookup(zobrist::Key pht_key) {
			return  pht_buffer + (pht_key % pht_buffer_len);
		}

		/**
		 * Locks the PHT.
		 */
		inline void lock() {
			pht_mutex.lock();
		}

		/**
		 * Unlocks the PHT.
		 */
		inline void unlock() {
			pht_mutex.unlock();
		}
	}
}
