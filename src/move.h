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
		 * Constructs a null move.
		 */
		Move();

		/**
		 * Constructs a move.
		 *
		 * @param src Source square.
		 * @param dst Destination square.
		 * @param ptype Promotion type.
		 */
		Move(int src, int dst, int ptype = piece::null);

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
		 * Shorthand for to_uci().
		 */
		operator std::string();

		bool operator==(const Move& rhs) const;
		bool operator!=(const Move& rhs) const;

		static const Move null;
	private:

		int m_src, m_dst, m_ptype;
	};

	struct PV {
		PV();
		std::string to_string();

		static constexpr int MAXSIZE = 128;

		int len;
		Move moves[PV::MAXSIZE];
	};
}
