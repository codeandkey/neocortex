/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "types.h"
#include "zobrist.h"

#include <assert.h>
#include <stdlib.h>

int NC_ZOBRIST_INITIALIZED = 0;
ncHashKey NC_ZOBRIST_PIECE_KEYS[64][12];
ncHashKey NC_ZOBRIST_CASTLE_KEYS[16];
ncHashKey NC_ZOBRIST_EP_KEYS[8];
ncHashKey NC_ZOBRIST_BTM_KEY;

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
	assert(!NC_ZOBRIST_INITIALIZED);

	for (ncSquare sq = 0; sq < 64; ++sq) {
		for (int p = 0; p < 12; ++p) {
			NC_ZOBRIST_PIECE_KEYS[sq][p] = randKey();
		}
	}

	for (int castle = 0; castle < 16; ++castle) {
		NC_ZOBRIST_CASTLE_KEYS[castle] = randKey();
	}

	for (int file = 0; file < 8; ++file) {
		NC_ZOBRIST_EP_KEYS[file] = randKey();
	}

	NC_ZOBRIST_BTM_KEY = randKey();
	NC_ZOBRIST_INITIALIZED = 1;
}
