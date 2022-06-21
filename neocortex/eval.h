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
