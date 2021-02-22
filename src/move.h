/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <string>

namespace neocortex {
	namespace move {
		constexpr int CASTLE_KS = 1 << 15;
		constexpr int CASTLE_QS = 1 << 16;
		constexpr int PAWN_JUMP = 1 << 17;
		constexpr int CAPTURE = 1 << 18;
		constexpr int CAPTURE_EP = 1 << 19;
		constexpr int PROMOTION = 1 << 20;

		/**
		 * Generates a new move from src to dst with optional ptype.
		 * 
		 * @param src Source square.
		 * @param dst Destination square.
		 * @param ptype Promotion type (optional)
		 * @param flags Flags to apply.
		 * 
		 * @return New move.
		 */
		inline int make(int src, int dst, int ptype = 0, int flags = 0) {
			return src | (dst << 6) | (ptype << 12) | flags;
		}

		/**
		 * Gets a null move.
		 * @return Null move.
		 */
		inline int null() {
			return -1;
		}

		/**
		 * Parses a move from UCI.
		 * Does not apply any flags, just sets the source and dest squares as well as the promotion type.
		 * Returns nullmove if UCI is invalid.
		 *
		 * @param uci Input uci.
		 */
		int from_uci(std::string uci);

		/**
		 * Converts a move to a UCI string.
		 *
		 * @param m Input move.
		 * @return UCI string.
		 */
		std::string to_uci(int m);

		/**
		 * Tests if this move is null.
		 *
		 * @param m Input move.
		 * @return true if this move is null.
		 */
		inline bool is_null(int m) {
			return m < 0;
		}

		/**
		 * Gets the move source square.
		 *
		 * @param m Input move.
		 * @return Source square.
		 */
		inline int src(int m) {
			return m & 0x3F;
		}

		/**
		 * Gets the move destination square.
		 *
		 * @param m Input move.
		 * @return Destination square.
		 */
		inline int dst(int m) {
			return (m >> 6) & 0x3F;
		}

		/**
		 * Gets the move promotion type.
		 *
		 * @param m Input move.
		 * @return Promotion type
		 */
		inline int ptype(int m) {
			return (m >> 12) & 0x7;
		}

		/**
		 * Tests if two moves are the same (excluding flags)
		 * 
		 * @param a First move.
		 * @param b Second move.
		 * @return true if moves have same coordinates and ptype.
		 */
		inline int match(int a, int b) {
			return src(a) == src(b) && dst(a) == dst(b) && ptype(a) == ptype(b);
		}
	}
}
