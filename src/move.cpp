/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "move.h"
#include "square.h"
#include "type.h"

#include <cassert>

using namespace neocortex;

int move::from_uci(std::string uci) {
	int ptype = 0;

	if (uci.size() == 5) {
		ptype = type::from_uci(uci.back());

		uci.pop_back();

		if (type::is_null(ptype)) {
			return move::null();
		}
	}

	if (uci.size() != 4) {
		return move::null();
	}

	return move::make(square::from_uci(uci.substr(0, 2)), square::from_uci(uci.substr(2, 2)), ptype);
}

std::string move::to_uci(int m) {
	if (move::is_null(m)) {
		return "0000";
	}

	std::string output = "";

	output += square::to_uci(move::src(m));
	output += square::to_uci(move::dst(m));

	if (m & move::PROMOTION) {
		output += type::to_uci(move::ptype(m));
	}

	return output;
}