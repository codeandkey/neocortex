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

// TODO - adjust endgame material weighting
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

static const int NC_EVAL_PHASE_TOTAL = \
	NC_EVAL_PHASE_VALS[0] * 16 +
	NC_EVAL_PHASE_VALS[1] * 4 +
	NC_EVAL_PHASE_VALS[2] * 4 +
	NC_EVAL_PHASE_VALS[3] * 4 +
	NC_EVAL_PHASE_VALS[4] * 2 +
	NC_EVAL_PHASE_VALS[5] * 2;

static const int NC_EVAL_TEMPO_BONUS = 4;
static const int NC_EVAL_CENTER_CONTROL = 1;
static const int NC_EVAL_KING_SAFETY = 7;
static const int NC_EVAL_PASSED_PAWNS = 11;
static const int NC_EVAL_ADV_PASSEDPAWN = 8;
static const int NC_EVAL_DEVELOPMENT = 15;
static const int NC_EVAL_FIRST_RANK_KING_MG = 7;
static const int NC_EVAL_PAWNS_PROT_KING_MG = 2;
static const int NC_EVAL_EDGE_KNIGHTS = -3;
static const int NC_EVAL_ISOLATED_PAWNS = -8;
static const int NC_EVAL_BACKWARD_PAWNS = -8;
static const int NC_EVAL_DOUBLED_PAWNS = -6;
static const int NC_EVAL_PAWN_CHAIN = 2;

static const int NC_EVAL_OPEN_FILE_ROOK = 8;
static const int NC_EVAL_OPEN_FILE_QUEEN = 4;

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
