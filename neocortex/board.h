/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "types.h"
#include "zobrist.h"

typedef struct {
	ncBitboard global_occ, color_occ[2], piece_occ[6];
	int mat_mg, mat_eg;
	ncPiece state[64];
	ncHashKey key;
} ncBoard;

int ncBoardFromFen(ncBoard* b, const char* fen);
void ncBoardStandard(ncBoard* b);
void ncBoardPlace(ncBoard* b, ncSquare sq, ncPiece pc);
ncPiece ncBoardRemove(ncBoard* b, ncSquare sq);
ncPiece ncBoardReplace(ncBoard* b, ncSquare sq, ncPiece pc);
int ncBoardToFen(ncBoard* b, char* dst, int max);
int ncBoardWritePretty(ncBoard* b, char* dst, int max);
ncBitboard ncBoardAttackers(ncBoard* b, ncSquare sq);
int ncBoardGuard(ncBoard* b, ncSquare sq);
int ncBoardIsAttacked(ncBoard* b, ncBitboard mask, ncColor col);
ncBitboard ncBoardPassers(ncBoard* b, ncColor color);
ncBitboard ncBoardAllspans(ncBoard* b, ncColor color);
ncBitboard ncBoardFrontspans(ncBoard* b, ncColor color);
ncBitboard ncBoardAttackspans(ncBoard* b, ncColor color);
ncBitboard ncBoardIsolated(ncBoard* b, ncColor color);
ncBitboard ncBoardBackward(ncBoard* b, ncColor color);
static inline int ncBoardMaterialMG(ncBoard* b)
{
	return b->mat_mg;
}

static inline int ncBoardMaterialEG(ncBoard* b)
{
	return b->mat_eg;
}

static inline ncPiece ncBoardGetPiece(ncBoard* b, ncSquare sq)
{
	assert(ncSquareValid(sq));
	return b->state[sq];
}

static inline ncBitboard ncBoardGlobalOcc(ncBoard* b)
{
	return b->global_occ;
}

static inline ncHashKey ncBoardHashKey(ncBoard* b)
{
	return b->key;
}

static inline ncBitboard ncBoardColorOcc(ncBoard* b, ncColor color)
{
	assert(ncColorValid(color));
	return b->color_occ[color];
}

static inline ncBitboard ncBoardPieceOcc(ncBoard* b, ncPiece piece_type)
{
	assert(ncPieceTypeValid(piece_type));
	return b->piece_occ[piece_type];
}

