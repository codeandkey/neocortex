/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "types.h"

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
ncHashKey ncZobristPiece(ncSquare sq, ncPiece piece);

/**
 * Gets the Zobrist key for a castle rights key.
 *
 * @param rights Castle rights.
 * @return Associated Zobrist key.
 */
ncHashKey ncZobristCastle(int rights);

/**
 * Gets the zobrist key for an en-passant state.
 *
 * @param file Square for en-passant.
 * @return Associated Zobrist key.
 */
ncHashKey ncZobristEnPassant(ncSquare sq);

/**
 * Gets the black-to-move Zobrist key.
 *
 * @return Zobrist key.
 */
ncHashKey ncZobristBlackToMove();
