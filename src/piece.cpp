/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "piece.h"
#include "util.h"

#include <cassert>
#include <cctype>

#include <stdexcept>

using namespace pine;

int piece::make_piece(int color, int type) {
	assert(color == WHITE || color == BLACK);
	assert(type >= 0 && type <= 5);

	return (type << 1) | color;
}

int piece::color(int piece) {
	assert(is_valid(piece));

	return piece & 1;
}

int piece::type(int piece) {
	assert(is_valid(piece));

	return piece >> 1;
}

bool piece::is_valid(int piece) {
	return piece >= 0 && piece < 12;
}

bool piece::is_type(int type) {
	return type >= 0 && type < 6;
}

char piece::get_uci(int piece) {
	assert(is_valid(piece));

	return "PpBbNnRrQqKk"[piece];
}

int piece::from_uci(char uci) {
	char typechar = tolower(uci);
	int type;

	switch (typechar) {
	case 'p':
		type = PAWN;
		break;
	case 'b':
		type = BISHOP;
		break;
	case 'n':
		type = KNIGHT;
		break;
	case 'r':
		type = ROOK;
		break;
	case 'q':
		type = QUEEN;
		break;
	case 'k':
		type = KING;
		break;
	default:
		throw util::fmterr("Invalid UCI piece: %c", uci);
	}

	return make_piece((uci == typechar) ? BLACK : WHITE, type);
}

int piece::color_from_uci(char uci) {
	switch (uci) {
	case 'w':
		return WHITE;
	case 'b':
		return BLACK;
	default:
		throw util::fmterr("Invalid UCI color: %c", uci);
	}
}

char piece::color_to_uci(int col) {
	assert(col == WHITE || col == BLACK);

	return "wb"[col];
}

int piece::type_from_uci(char uci) {
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
		throw util::fmterr("Invalid UCI type: %c", uci);
	}
}

char piece::type_to_uci(int type) {
	assert(type >= 0 && type < 6);

	return "pbnrqk"[type];
}