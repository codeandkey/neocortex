/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "types.h"

extern int NC_MATERIAL_MG[];
extern int NC_MATERIAL_EG[];
extern int NC_GUARD[];

extern int NC_EVAL_PHASE_VALS[];
extern int NC_EVAL_PHASE_TOTAL;

extern int NC_EVAL_CENTER_CONTROL_MG;
extern int NC_EVAL_CENTER_CONTROL_EG;
extern int NC_EVAL_KING_SAFETY_MG;
extern int NC_EVAL_KING_SAFETY_EG;
extern int NC_EVAL_PASSED_PAWNS_MG;
extern int NC_EVAL_PASSED_PAWNS_EG;
extern int NC_EVAL_ADV_PASSEDPAWN_MG;
extern int NC_EVAL_ADV_PASSEDPAWN_EG;
extern int NC_EVAL_DEVELOPMENT_MG;
extern int NC_EVAL_DEVELOPMENT_EG;
extern int NC_EVAL_FIRST_RANK_KING_MG;
extern int NC_EVAL_FIRST_RANK_KING_EG;
extern int NC_EVAL_PAWNS_PROT_KING_MG;
extern int NC_EVAL_PAWNS_PROT_KING_EG;
extern int NC_EVAL_EDGE_KNIGHTS_MG;
extern int NC_EVAL_EDGE_KNIGHTS_EG;
extern int NC_EVAL_ISOLATED_PAWNS_MG;
extern int NC_EVAL_ISOLATED_PAWNS_EG;
extern int NC_EVAL_BACKWARD_PAWNS_MG;
extern int NC_EVAL_BACKWARD_PAWNS_EG;
extern int NC_EVAL_DOUBLED_PAWNS_MG;
extern int NC_EVAL_DOUBLED_PAWNS_EG;
extern int NC_EVAL_PAWN_CHAIN_MG;
extern int NC_EVAL_PAWN_CHAIN_EG;
extern int NC_EVAL_OPEN_FILE_ROOK_MG;
extern int NC_EVAL_OPEN_FILE_ROOK_EG;
extern int NC_EVAL_OPEN_FILE_QUEEN_MG;
extern int NC_EVAL_OPEN_FILE_QUEEN_EG;

/**
 * Returns the MG material score for <pc>. Negative values are returned for
 * black material, and positive values for white.
 *
 * @param pc Piece to evaluate.
 * @return Middlegame material value.
 */
static inline int ncMaterialValueMG(ncPiece pc)
{
	assert(ncPieceValid(pc));
	return NC_MATERIAL_MG[pc];
}

/**
 * Returns the EG material score for <pc>. Negative values are returned for
 * black material, and positive values for white.
 *
 * @param pc Piece to evaluate.
 * @return Middlegame material value.
 */
static inline int ncMaterialValueEG(ncPiece pc)
{
	assert(ncPieceValid(pc));
	return NC_MATERIAL_EG[pc];
}

static inline int ncGuardValue(ncPiece pc)
{
	assert(ncPieceValid(pc));
	return NC_GUARD[pc];
}

int ncEvalNumOptions();
const char* ncEvalOptionStr(int ind);
int ncEvalSetOption(const char* name, int value);
