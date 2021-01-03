/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "eval.h"
#include "eval_consts.h"
#include "util.h"

using namespace neocortex;

bool score::is_mate(int value) {
	return (value >= CHECKMATE - MATE_THRESHOLD || value <= CHECKMATED + MATE_THRESHOLD);
}

int score::parent(int value) {
	if (value == INCOMPLETE) {
		return value;
	}

	if (is_mate(value)) {
		if (value < 0) return value + 1;
		if (value > 0) return value - 1;

		return 0;
	} else {
		return value;
	}
}

std::string score::to_string(int value) {
	if (is_mate(value)) {
		if (value > 0) {
			return util::format("#%d", CHECKMATE - value);
		} else {
			return util::format("#-%d", value - CHECKMATED);
		}
	} else {
		return util::format("%+.2f", value / 100.0f);
	}
}

std::string score::to_uci(int value) {
	if (is_mate(value)) {
		if (value > 0) {
			return util::format("mate %d", CHECKMATE - value);
		} else {
			return util::format("mate -%d", value - CHECKMATED);
		}
	} else {
		return util::format("cp %d", value);
	}
}
