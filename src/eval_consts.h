#pragma once

/*
 * Constants altering behavior of static evaluation.
 */

#include "eval.h"

/* This eval is returned as a static score, not the actual value for a draw. */
#define NC_EVAL_DRAW NC_EVAL_MAX

/*
 * Material values.
 */


#define NC_EVAL_PAWN_MG 100
#define NC_EVAL_KNIGHT_MG 300
#define NC_EVAL_BISHOP_MG 300
#define NC_EVAL_ROOK_MG 500
#define NC_EVAL_QUEEN_MG 900

#define NC_EVAL_PAWN_EG 150
#define NC_EVAL_KNIGHT_EG 300
#define NC_EVAL_BISHOP_EG 300
#define NC_EVAL_ROOK_EG 500
#define NC_EVAL_QUEEN_EG 900

#define NC_EVAL_MOBILITY 2
#define NC_EVAL_CONTEMPT 0
#define NC_EVAL_PAWN_STRUCTURE 30
