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

int ncBoardReadFen(ncBoard* b, const char* fen);
void ncBoardStandard(ncBoard* b);
void ncBoardPlace(ncBoard* b, ncSquare sq, ncPiece pc);
ncPiece ncBoardRemove(ncBoard* b, ncSquare sq);
ncPiece ncBoardReplace(ncBoard* b, ncSquare sq, ncPiece pc);
int ncBoardWriteFen(ncBoard* b, char* dst, int max);
int ncBoardWritePretty(ncBoard* b, char* dst, int max);
ncBitboard ncBoardGlobalOcc(ncBoard* b);
ncBitboard ncBoardColorOcc(ncBoard* b, ncColor color);
ncBitboard ncBoardPieceOcc(ncBoard* b, ncPiece piece_type);
ncPiece ncBoardGetPiece(ncBoard* b, ncSquare sq);
ncHashKey ncBoardHashKey(ncBoard* b);
ncBitboard ncBoardAttackers(ncBoard* b, ncSquare sq);
int ncBoardGuard(ncBoard* b, ncSquare sq);
int ncBoardIsAttacked(ncBoard* b, ncSquare sq);
ncBitboard ncBoardPassers(ncBoard* b, ncColor color);
ncBitboard ncBoardAllspans(ncBoard* b, ncColor color);
ncBitboard ncBoardFrontspans(ncBoard* b, ncColor color);
ncBitboard ncBoardAttackspans(ncBoard* b, ncColor color);
ncBitboard ncBoardIsolated(ncBoard* b, ncColor color);
ncBitboard ncBoardBackward(ncBoard* b, ncColor color);
int ncBoardMaterialMG(ncBoard* b);
int ncBoardMaterialEG(ncBoard* b);
