#pragma once

/*
 * Represents a single game position.
 */

#include "bb.h"
#include "move.h"
#include "square.h"
#include "zobrist.h"
#include "eval.h"
#include "pst.h"

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
    int castling, check;
    nc_square ep_target;
    int halfmove_clock;
    nc_piece captured;
    nc_square captured_at;
    nc_move lastmove;
    nc_zkey key; /* history key, stored for detecting repetition */
    nc_bb attacks[2]; /* attack bitboards */
} nc_pstate;

typedef struct {
    nc_pstate states[NC_MAX_PLY];
    int ply;

    nc_zkey key;
    nc_color color_to_move;

    /* Incremental evaluation (NOT from the perspective of CTM) */
    nc_pst_eval pst;

    /* Occupancy bitboards */
    nc_bb color[2], piece[6], global;

    /* Piece array */
    nc_piece board[64];
} nc_position;

/* Standard position init */
void nc_position_init(nc_position* dst);

/* FEN parsing */
void nc_position_init_fen(nc_position* dst, const char* fen);

/* Position move interface */
int nc_position_make_move(nc_position* dst, nc_move move);
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
void nc_position_dump(nc_position* p, FILE* out, int include_moves);

/* Scoring */
nc_eval nc_position_score(nc_position* dst, nc_movelist* out);
nc_eval nc_position_score_thin(nc_position* dst);
float nc_position_phase(nc_position* dst);
int nc_position_pawn_structure(nc_position* dst, nc_color col);

/* Repetition detection */
int nc_position_is_repetition(nc_position* dst);

/* Position examining */
static inline int nc_position_is_quiet(nc_position* p) {
    return (p->states[p->ply].captured == NC_PIECE_NULL) && !p->states[p->ply].check;
}
