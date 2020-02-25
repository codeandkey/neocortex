#pragma once

/*
 * Represents a single game position.
 */

#include "move.h"
#include "square.h"
#include "zobrist.h"

/* Castle right flags */
#define NC_WHITE_QUEENSIDE 1
#define NC_WHITE_KINGSIDE  2
#define NC_BLACK_QUEENSIDE 4
#define NC_BLACK_KINGSIDE  8

/* Position configuration */
#define NC_MAX_PLY 512

typedef struct {
    int castling;
    nc_square ep_target;
    int halfmove_clock;
} nc_pstate;

typedef struct {
    nc_pstate states[NC_MAX_PLY];
    int ply;

    nc_zkey key;
    nc_color color_to_move;
} nc_position;

void nc_position_init(nc_position* dst);
