/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "types.h"

#include <assert.h>
#include <stdlib.h>

static int zobrist_initialized = 0;
static ncHashKey piece_keys[64][12], castle_keys[16], en_passant_keys[8], black_to_move_key;

/**
 * Generates a random Zobrist hash key.
 *
 * @return The generated key.
 */
static ncHashKey randKey()
{
	uint8_t val[8];

	for (int i = 0; i < 8; ++i)
		val[i] = rand() & 0xFF;

	return *((ncHashKey*) val);
}

void ncZobristInit() {
	assert(!zobrist_initialized);

	for (ncSquare sq = 0; sq < 64; ++sq) {
		for (int p = 0; p < 12; ++p) {
			piece_keys[sq][p] = randKey();
		}
	}

	for (int castle = 0; castle < 16; ++castle) {
		castle_keys[castle] = randKey();
	}

	for (int file = 0; file < 8; ++file) {
		en_passant_keys[file] = randKey();
	}

	black_to_move_key = randKey();
	zobrist_initialized = 1;
}

ncHashKey ncZobristPiece(ncSquare sq, int p) {
	assert(zobrist_initialized);
	assert(ncPieceValid(p));
	assert(ncSquareValid(sq));

	return piece_keys[sq][p];
}

ncHashKey ncZobristCastle(int rights) {
	assert(zobrist_initialized);
	return castle_keys[rights];
}

ncHashKey ncZobristEnPassant(ncSquare sq) {
	assert(zobrist_initialized);
	assert(ncSquareValid(sq));

	return en_passant_keys[ncSquareFile(sq)];
}

ncHashKey ncZobristBlackToMove() {
	assert(zobrist_initialized);
	return black_to_move_key;
}
