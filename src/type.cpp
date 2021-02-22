/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "type.h"

using namespace neocortex;

int type::from_uci(char uci) {
	switch (uci) {
	case 'p':
		return PAWN;
	case 'b':
		return BISHOP;
	case 'n':
		return KNIGHT;
	case 'r':
		return ROOK;
	case 'q':
		return QUEEN;
	case 'k':
		return KING;
	default:
		return null();
	}
}

char type::to_uci(int t) {
	if (is_null(t) || t > 5) return '?';
	return "pbnrqk"[t];
}