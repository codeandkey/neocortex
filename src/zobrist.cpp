/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "piece.h"
#include "square.h"
#include "zobrist.h"

#include <cassert>
#include <random>

using namespace neocortex;

static bool zobrist_initialized = false;
static zobrist::Key piece_keys[64][12], castle_keys[16], en_passant_keys[8], black_to_move_key;

void zobrist::init() {
	assert(!zobrist_initialized);

	std::random_device rng;
	std::mt19937_64 twister(rng()); /* TODO: tune a good zobrist seed */
	std::uniform_int_distribution<zobrist::Key> dist;

	for (int sq = 0; sq < 64; ++sq) {
		for (int p = 0; p < 12; ++p) {
			piece_keys[sq][p] = dist(twister);
		}
	}

	for (int castle = 0; castle < 16; ++castle) {
		castle_keys[castle] = dist(twister);
	}

	for (int file = 0; file < 8; ++file) {
		en_passant_keys[file] = dist(twister);
	}

	black_to_move_key = dist(twister);
	zobrist_initialized = true;
}

bool is_init() {
	return zobrist_initialized;
}

zobrist::Key zobrist::piece(int sq, int p) {
	assert(zobrist_initialized);
	if (p == piece::null) return 0;
	return piece_keys[sq][p];
}

zobrist::Key zobrist::castle(int rights) {
	assert(zobrist_initialized);
	return castle_keys[rights];
}

zobrist::Key zobrist::en_passant(int sq) {
	assert(zobrist_initialized);
	if (sq == square::null) return 0;
	return en_passant_keys[square::file(sq)];
}

zobrist::Key zobrist::black_to_move() {
	assert(zobrist_initialized);
	return black_to_move_key;
}
