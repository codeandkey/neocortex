#pragma once

/*
 * Pseudolegal move generation functions.
 */

#include "bb.h"
#include "move.h"
#include "square.h"
#include "position.h"

#define NC_MOVEGEN_STAGE_BEGIN 0
#define NC_MOVEGEN_STAGE_PV_MOVE 1
#define NC_MOVEGEN_STAGE_CAPTURES 2
#define NC_MOVEGEN_STAGE_CASTLES 3
#define NC_MOVEGEN_STAGE_QUIETS 4
#define NC_MOVEGEN_STAGE_END 5

typedef struct {
    int capture_victim, capture_attacker; /* MVV-LVA staging */
    int stage, ind;
    nc_movelist moves;
} nc_movegen;

/**
 * Start staged move generation position.
 * The current stage is stored at *stage which must be non-null.
 *
 * @param pos Target position.
 * @param state Movegen state.
 */
void nc_movegen_start_gen(nc_position* pos, nc_movegen* state);

/**
 * Loads the next pseudolegal move for pos if there is one.
 * Returns 0 if there are no moves left.
 *
 * @param pos Target position.
 * @param state Current state.
 * @param out Output move.
 * @return 1 if a move was loaded, 0 otherwise.
 */
int nc_movegen_next_move(nc_position* pos, nc_movegen* state, nc_move* out);

/**
 * Specialized method to generate an attacked square bitboard for a color.
 * @param pos Target position.
 * @param col Source piece color.
 * @return New attack bitboard.
 */
nc_bb nc_movegen_attacked_squares(nc_position* pos, nc_color col);

/**
 * Generic method for quickly testing if a square is attacked.
 * Core algorithm for check testing.
 *
 * @param pos Position to test.
 * @param sq Square to test.
 * @param col Attacking color.
 */
int nc_movegen_square_is_attacked(nc_position* pos, nc_square sq, nc_color col);

/**
 * Slightly modified square attack method for testing if a mask is attacked.
 *
 * @param pos Position to test.
 * @param sq Square to test.
 * @param col Attacking color.
 */
int nc_movegen_mask_is_attacked(nc_position* pos, nc_bb m, nc_color col);

/**
 * Specialized method to test if a king is in check. This works by castling attack lines
 * out from the king instead of generating the whole attack map and is MUCH more efficient
 * than computing the whole attack map and testing intersections.
 *
 * @param pos Position to test.
 * @param col King color to test.
 * @return 1 if the king is in check, 0 otherwise
 */
int nc_movegen_get_king_in_check(nc_position* p, nc_color col);
