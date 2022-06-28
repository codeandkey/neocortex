/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "types.h"

static const int NC_MATERIAL_MG[] =
{
	100, -100,
	300, -300,
	300, -300,
	500, -500,
	900, -900,
	1200, -1200
};

static const int NC_MATERIAL_EG[] =
{
	100, -100,
	300, -300,
	300, -300,
	500, -500,
	900, -900,
	1200, -1200
};

static const int NC_GUARD[] =
{
	9, -9,
	6, -6,
	5, -5,
	2, -2,
	1, -1,
	1, -1
};

static const int NC_EVAL_PHASE_VALS[6] = {0, 1, 1, 2, 4, 0};
static const int NC_EVAL_PHASE_TOTAL = 4 + 4 + 8 + 8;

#ifndef NC_EVAL_CENTER_CONTROL_MG
#define NC_EVAL_CENTER_CONTROL_MG 20
#endif

#ifndef NC_EVAL_CENTER_CONTROL_EG
#define NC_EVAL_CENTER_CONTROL_EG 8
#endif

#ifndef NC_EVAL_KING_SAFETY_MG
#define NC_EVAL_KING_SAFETY_MG 7
#endif

#ifndef NC_EVAL_KING_SAFETY_EG
#define NC_EVAL_KING_SAFETY_EG 6
#endif

#ifndef NC_EVAL_PASSED_PAWNS_MG
#define NC_EVAL_PASSED_PAWNS_MG 15
#endif

#ifndef NC_EVAL_PASSED_PAWNS_EG
#define NC_EVAL_PASSED_PAWNS_EG 30
#endif

#ifndef NC_EVAL_ADV_PASSEDPAWN_MG
#define NC_EVAL_ADV_PASSEDPAWN_MG 8
#endif

#ifndef NC_EVAL_ADV_PASSEDPAWN_EG
#define NC_EVAL_ADV_PASSEDPAWN_EG 8
#endif

#ifndef NC_EVAL_DEVELOPMENT_MG
#define NC_EVAL_DEVELOPMENT_MG 35
#endif

#ifndef NC_EVAL_DEVELOPMENT_EG
#define NC_EVAL_DEVELOPMENT_EG 20
#endif

#ifndef NC_EVAL_FIRST_RANK_KING_MG
#define NC_EVAL_FIRST_RANK_KING_MG 10
#endif

#ifndef NC_EVAL_FIRST_RANK_KING_EG
#define NC_EVAL_FIRST_RANK_KING_EG -10
#endif

#ifndef NC_EVAL_PAWNS_PROT_KING_MG
#define NC_EVAL_PAWNS_PROT_KING_MG 8
#endif

#ifndef NC_EVAL_PAWNS_PROT_KING_EG
#define NC_EVAL_PAWNS_PROT_KING_EG 8
#endif

#ifndef NC_EVAL_EDGE_KNIGHTS_MG
#define NC_EVAL_EDGE_KNIGHTS_MG -10
#endif

#ifndef NC_EVAL_EDGE_KNIGHTS_EG
#define NC_EVAL_EDGE_KNIGHTS_EG -5
#endif

#ifndef NC_EVAL_ISOLATED_PAWNS_MG
#define NC_EVAL_ISOLATED_PAWNS_MG -10
#endif

#ifndef NC_EVAL_ISOLATED_PAWNS_EG
#define NC_EVAL_ISOLATED_PAWNS_EG -10
#endif

#ifndef NC_EVAL_BACKWARD_PAWNS_MG
#define NC_EVAL_BACKWARD_PAWNS_MG -10
#endif

#ifndef NC_EVAL_BACKWARD_PAWNS_EG
#define NC_EVAL_BACKWARD_PAWNS_EG -10
#endif

#ifndef NC_EVAL_DOUBLED_PAWNS_MG
#define NC_EVAL_DOUBLED_PAWNS_MG -10
#endif

#ifndef NC_EVAL_DOUBLED_PAWNS_EG
#define NC_EVAL_DOUBLED_PAWNS_EG -20
#endif

#ifndef NC_EVAL_PAWN_CHAIN_MG
#define NC_EVAL_PAWN_CHAIN_MG 4
#endif

#ifndef NC_EVAL_PAWN_CHAIN_EG
#define NC_EVAL_PAWN_CHAIN_EG 4
#endif

#ifndef NC_EVAL_OPEN_FILE_ROOK_MG
#define NC_EVAL_OPEN_FILE_ROOK_MG 5
#endif

#ifndef NC_EVAL_OPEN_FILE_ROOK_EG
#define NC_EVAL_OPEN_FILE_ROOK_EG 5
#endif

#ifndef NC_EVAL_OPEN_FILE_QUEEN_MG
#define NC_EVAL_OPEN_FILE_QUEEN_MG 5
#endif

#ifndef NC_EVAL_OPEN_FILE_QUEEN_EG
#define NC_EVAL_OPEN_FILE_QUEEN_EG 5
#endif

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
