#pragma once

/*
 * Represents a single game position.
 */

#include "bb.h"
#include "move.h"
#include "square.h"
#include "zobrist.h"

#include <stdlib.h>

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
    nc_piece captured;
    nc_square captured_at;
    nc_move lastmove;
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

/* Standard position init */
void nc_position_init(nc_position* dst);

/* Position move interface */
void nc_position_make_move(nc_position* dst, nc_move move);
void nc_position_unmake_move(nc_position* dst, nc_move move);

/* State transition helpers */
void nc_position_update_castling(nc_position* dst, int old, int next);
void nc_position_update_ep_target(nc_position* dst, nc_square old, nc_square target);

/* Board manipulation */
void nc_position_place_piece(nc_position* dst, nc_piece p, nc_square at);
void nc_position_replace_piece(nc_position* dst, nc_piece p, nc_square at);
void nc_position_move_piece(nc_position* dst, nc_square from, nc_square to);
nc_piece nc_position_remove_piece(nc_position* dst, nc_square at);
nc_piece nc_position_capture_piece(nc_position* dst, nc_square from, nc_square to);

/* Occupancy manipulation */
void nc_position_flip_piece(nc_position* dst, nc_piece p, nc_square at);

/* Debugging */
void nc_position_dump(nc_position* p, FILE* out);

/* Move generation */
void nc_position_legal_moves(nc_position* dst, nc_movelist* out);

/* Specific pseudolegal move generation. Used to influence move ordering */
void nc_position_gen_castles(nc_position* dst, nc_movelist* out);
void nc_position_gen_captures(nc_position* dst, nc_movelist* out);
void nc_position_gen_quiets(nc_position* dst, nc_movelist* out);

void nc_position_gen_pawn_moves(nc_position* dst, nc_movelist* out, int captures);
void nc_position_gen_rook_moves(nc_position* dst, nc_movelist* out, int captures);
void nc_position_gen_knight_moves(nc_position* dst, nc_movelist* out, int captures);
void nc_position_gen_bishop_moves(nc_position* dst, nc_movelist* out, int captures);
void nc_position_gen_queen_moves(nc_position* dst, nc_movelist* out, int captures);
void nc_position_gen_king_moves(nc_position* dst, nc_movelist* out, int captures);

int nc_position_test_mask_is_attacked(nc_position* dst, nc_bb mask, nc_color by);
