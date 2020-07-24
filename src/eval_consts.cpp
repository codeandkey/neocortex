#include "eval_consts.h"

using namespace pine;

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

