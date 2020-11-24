/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "eval_consts.h"
#include "piece.h"

using namespace neocortex;

const int eval::MATERIAL_MG[6] = {
	100,
	300,
	300,
	500,
	900,
	0
};

const int eval::MATERIAL_EG[6] = {
	150,
	300,
	300,
	500,
	900,
	0
};

const int eval::MATERIAL_MG_MAX = \
	20 * eval::MATERIAL_MG[0] + \
	4 * eval::MATERIAL_MG[1] + \
	4 * eval::MATERIAL_MG[2] + \
	4 * eval::MATERIAL_MG[3] + \
	2 * eval::MATERIAL_MG[4] + \
	2 * eval::MATERIAL_MG[5];


const int eval::MATERIAL_EG_MAX = \
	20 * eval::MATERIAL_EG[0] + \
	4 * eval::MATERIAL_EG[1] + \
	4 * eval::MATERIAL_EG[2] + \
	4 * eval::MATERIAL_EG[3] + \
	2 * eval::MATERIAL_EG[4] + \
	2 * eval::MATERIAL_EG[5];

const int eval::MATERIAL_MG_LOOKUP[12] = {
		eval::MATERIAL_MG[piece::PAWN], -eval::MATERIAL_MG[piece::PAWN],
		eval::MATERIAL_MG[piece::BISHOP], -eval::MATERIAL_MG[piece::BISHOP],
		eval::MATERIAL_MG[piece::KNIGHT], -eval::MATERIAL_MG[piece::KNIGHT],
		eval::MATERIAL_MG[piece::ROOK], -eval::MATERIAL_MG[piece::ROOK],
		eval::MATERIAL_MG[piece::QUEEN], -eval::MATERIAL_MG[piece::QUEEN],
		eval::MATERIAL_MG[piece::KING], -eval::MATERIAL_MG[piece::KING],
};

const int eval::MATERIAL_EG_LOOKUP[12] = {
	eval::MATERIAL_MG[piece::PAWN], -eval::MATERIAL_MG[piece::PAWN],
	eval::MATERIAL_MG[piece::BISHOP], -eval::MATERIAL_MG[piece::BISHOP],
	eval::MATERIAL_MG[piece::KNIGHT], -eval::MATERIAL_MG[piece::KNIGHT],
	eval::MATERIAL_MG[piece::ROOK], -eval::MATERIAL_MG[piece::ROOK],
	eval::MATERIAL_MG[piece::QUEEN], -eval::MATERIAL_MG[piece::QUEEN],
	eval::MATERIAL_MG[piece::KING], -eval::MATERIAL_MG[piece::KING],
};

const int eval::PHASE_VALS[6] = {
	0,
	1,
	1,
	2,
	4,
	0,
};

const int eval::GUARD_VALUES[12] = {
	9, -9, 5, -5, 6, -6, 2, -2, 1, -1, 1, -1
};

const int eval::PHASE_TOTAL = 16 * eval::PHASE_VALS[0] + 4 * (eval::PHASE_VALS[1] + eval::PHASE_VALS[2] + eval::PHASE_VALS[3]) + 2 * eval::PHASE_VALS[4];