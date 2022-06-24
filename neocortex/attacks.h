/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "types.h"
#include "magic_consts.h"

#include <assert.h>

extern ncBitboard NC_KING_ATTACKS[64], NC_KNIGHT_ATTACKS[64], NC_PAWN_ATTACKS[2][64];
extern ncBitboard* NC_ROOK_ATTACKS[64], *NC_BISHOP_ATTACKS[64];
extern ncBitboard NC_PAWN_FRONTSPANS[2][64];
extern ncBitboard NC_PAWN_ATTACKSPANS[2][64];

/**
 * Initializes the attack lookup tables.
 * Must be called before performing any lookups.
 * Failure to do so will result in undefined behavior.
 */
void ncAttacksInit();

/**
 * Computes a magic attack index from relevant occupancy, magic number and bits.
 * Used internally for sliding piece attack lookups.
 *
 * @param rocc Relevant occupancy from sliding piece.
 * @param magic Magic number. (see magic_consts)
 * @param bits Number if relevant occupancy bits.
 *
 * @return Attack index.
 */
static inline int magic_index(ncBitboard rocc, ncBitboard magic, int bits) {
	return (int) ((rocc * magic) >> (64 - bits));
}

/**
 * Looks up pawn attacks from a source square.
 *
 * @param col Attacking color.
 * @param sq Source square.
 * @return Bitboard of attacked squares.
 */
static inline ncBitboard ncAttacksPawn(int col, int sq) {
	return NC_PAWN_ATTACKS[col][sq];
}

/**
 * Looks up pawn frontspans from a source square.
 *
 * @param col Attacking color.
 * @param sq Source square.
 * @return Bitboard of squares the pawn can advance to.
 */
static inline ncBitboard ncPawnFrontspans(int col, int sq) {
	return NC_PAWN_FRONTSPANS[col][sq];
}

/**
 * Looks up pawn attackspans from a source square.
 *
 * @param col Attacking color.
 * @param sq Source square.
 * @return Bitboard of squares the pawn can eventually attack.
 */
static inline ncBitboard ncPawnAttackspans(int col, int sq) {
	return NC_PAWN_ATTACKSPANS[col][sq];
}

/**
 * Looks up king attacks from a source square.
 *
 * @param sq Source square.
 * @return Bitboard of attacked squares.
 */
static inline ncBitboard ncAttacksKing(int sq) {
	return NC_KING_ATTACKS[sq];
}

/**
 * Looks up knight attacks from a source square.
 *
 * @param sq Source square.
 * @return Bitboard of attacked squares.
 */
static inline ncBitboard ncAttacksKnight(int sq) {
	assert(ncSquareValid(sq));
	return NC_KNIGHT_ATTACKS[sq];
}

/**
 * Looks up bishop attacks from a source square and occupancy.
 *
 * @param sq Source square.
 * @param occ Board global occupancy.
 * @return Bitboard of attacked squares.
 */
static inline ncBitboard ncAttacksBishop(int sq, ncBitboard occ) {
	assert(ncSquareValid(sq));
	return NC_BISHOP_ATTACKS[sq][magic_index(occ & NC_BISHOP_MASKS[sq], NC_BISHOP_MAGICS[sq], NC_BISHOP_BITS[sq])];
}

/**
 * Looks up rook attacks from a source square and occupancy.
 *
 * @param sq Source square.
 * @param occ Board global occupancy.
 * @return Bitboard of attacked squares.
 */
static inline ncBitboard ncAttacksRook(int sq, ncBitboard occ) {
	assert(ncSquareValid(sq));
	return NC_ROOK_ATTACKS[sq][magic_index(occ & NC_ROOK_MASKS[sq], NC_ROOK_MAGICS[sq], NC_ROOK_BITS[sq])];
}

/**
 * Looks up queen attacks from a source square and occupancy.
 *
 * @param sq Source square.
 * @param occ Board global occupancy.
 * @return Bitboard of attacked squares.
 */
static inline ncBitboard ncAttacksQueen(int sq, ncBitboard occ) {
	assert(ncSquareValid(sq));
	return ncAttacksRook(sq, occ) | ncAttacksBishop(sq, occ);
}
