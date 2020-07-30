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

namespace pine {
	class Move {
	public:
		Move(int value = null);
		Move(int src, int dst, int flags = 0, int ptype = piece::PAWN);
		Move(std::string uci);  /* note: will not add any flags except promotion */

		std::string to_uci();
		std::string to_pgn(Board& context);
		bool is_valid();
		bool match_uci(std::string uci);

		int src();
		int dst();
		int ptype();

		Move& set(int flag);
		bool get(int flag);

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