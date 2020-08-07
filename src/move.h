/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "board.h"
#include "piece.h"
#include "square.h"

#include <string>

/* [0-5] src, [6-11] dst, [12-14] ptype, [15+] flags */

namespace neocortex {
	class Move {
	public:
		/**
		 * Constructs a move from an encoded value.
		 */
		Move(int value = null);

		/**
		 * Constructs a move.
		 *
		 * @param src Source square.
		 * @param dst Destination square.
		 * @param flags Move flags.
		 * @param ptype Promotion type.
		 */
		Move(int src, int dst, int flags = 0, int ptype = piece::PAWN);

		/**
		 * Parses a move from UCI.
		 * Does not apply any flags, just sets the source and dest squares as well as the promotion type.
		 *
		 * @param uci Input uci.
		 */
		Move(std::string uci);

		/**
		 * Converts a move to a UCI string.
		 *
		 * @return UCI string.
		 */
		std::string to_uci();

		/**
		 * Converts a move to PGN.
		 *
		 * @param context Current board context.
		 */
		std::string to_pgn(Board& context);

		/**
		 * Tests if this move is not null.
		 *
		 * @return true if this move is not null.
		 */
		bool is_valid();

		/**
		 * Tests if this move matches with a UCI string.
		 * This is used for matching input UCI moves to generated actual moves.
		 *
		 * @param uci Input uci.
		 * @return true if this move can be represented with the input.
		 */
		bool match_uci(std::string uci);

		/**
		 * Gets the move source square.
		 *
		 * @return Source square.
		 */
		int src();

		/**
		 * Gets the move destination square.
		 *
		 * @return Destination square.
		 */
		int dst();

		/**
		 * Gets the move promotion type, or null if there is none set.
		 *
		 * @return Promotion type, or null.
		 */
		int ptype();

		/**
		 * Sets a flag on this move.
		 *
		 * @param flag Flag to set.
		 * @return Reference to self.
		 */
		Move& set(int flag);

		/**
		 * Queries a flag on this move.
		 *
		 * @param flag Flag to get
		 * @return true if flag is set, false otherwise.
		 */
		bool get(int flag);

		/**
		 * Shorthand for to_uci().
		 */
		operator std::string();

		bool operator==(const Move& rhs);
		bool operator!=(const Move& rhs);

		static constexpr int null = -1;
		static constexpr int PAWN_JUMP = 1 << 15;
		static constexpr int PROMOTION = 1 << 16;
		static constexpr int CAPTURE = 1 << 17;
		static constexpr int CASTLE_KINGSIDE = 1 << 18;
		static constexpr int CASTLE_QUEENSIDE = 1 << 19;
	private:

		int value;
	};

	struct PV {
		PV();
		std::string to_string();

		static constexpr int MAXSIZE = 128;

		int len;
		Move moves[PV::MAXSIZE];
	};
}
