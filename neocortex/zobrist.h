/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "types.h"

extern int NC_ZOBRIST_INITIALIZED;
extern ncHashKey NC_ZOBRIST_PIECE_KEYS[64][12];
extern ncHashKey NC_ZOBRIST_CASTLE_KEYS[16];
extern ncHashKey NC_ZOBRIST_EP_KEYS[8];
extern ncHashKey NC_ZOBRIST_BTM_KEY;

/**
 * Initializes Zobrist keys.
 * Must be called before querying any keys.
 */
void ncZobristInit();

/**
 * Gets the Zobrist key for a piece on a square.
 *
 * @param sq Input square.
 * @param piece Input piece.
 * @return Zobrist key for piece on square.
 */
static inline ncHashKey ncZobristPiece(ncSquare sq, int p) {
    assert(NC_ZOBRIST_INITIALIZED);
    assert(ncPieceValid(p));
    assert(ncSquareValid(sq));

    return NC_ZOBRIST_PIECE_KEYS[sq][p];
}

/**
 * Gets the Zobrist key for a castle rights key.
 *
 * @param rights Castle rights.
 * @return Associated Zobrist key.
 */
static inline ncHashKey ncZobristCastle(int rights) {
    assert(rights >= 0 && rights < 16);

    return NC_ZOBRIST_CASTLE_KEYS[rights];
}

/**
 * Gets the zobrist key for an en-passant state.
 *
 * @param file Square for en-passant.
 * @return Associated Zobrist key.
 */
static inline ncHashKey ncZobristEnPassant(ncSquare sq)
{
	return NC_ZOBRIST_EP_KEYS[ncSquareFile(sq)];
}

/**
 * Gets the black-to-move Zobrist key.
 *
 * @return Zobrist key.
 */
static inline ncHashKey ncZobristBlackToMove()
{
	return NC_ZOBRIST_BTM_KEY;
}
