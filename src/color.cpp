/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "color.h"

using namespace neocortex;

int color::from_uci(char c) {
	switch (c) {
	case 'w':
		return WHITE;
	case 'b':
		return BLACK;
	default:
		return null();
	}
}

char color::to_uci(int col) {
	if (is_null(col) || col > 1) {
		return '?';
	}

	return "wb"[col];
}