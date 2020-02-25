#pragma once

/*
 * Represents a single game position.
 */

#include "bb.h"
#include "move.h"
#include "square.h"
#include "zobrist.h"

/* Castle right flags */
#define NC_WHITE_QUEENSIDE 1
#define NC_WHITE_KINGSIDE  2
#define NC_BLACK_QUEENSIDE 4
#define NC_BLACK_KINGSIDE  8
#define NC_CASTLE_ALL 0xF

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

    /* Occupancy bitboards */
    nc_bb color[2], piece[6], global;

    /* Piece array */
    nc_piece board[64];
} nc_position;

void nc_position_init(nc_position* dst);
void nc_position_make_move(nc_position* dst, nc_move move);
void nc_position_unmake_move(nc_position* dst, nc_move move);
void nc_position_legal_moves(nc_position* dst, nc_movelist* out);

/* Updates the current ply and zkey. */
void nc_position_update_castling(nc_position* dst, int castling);

void nc_position_place_piece(nc_position* dst, nc_piece p, nc_square at);
void nc_position_move_piece(nc_position* dst, nc_square from, nc_square to);
nc_piece nc_position_capture_piece(nc_position* dst, nc_square from, nc_square to);

void nc_position_flip_piece(nc_position* dst, nc_piece p, nc_square at);
