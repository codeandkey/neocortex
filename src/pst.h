#pragma once

/*
 * Evaluation PST
 *
 * PST evaluations maintain two scores -- one for opening and one for endgame
 * they must be tracked together to maintain the ability to incrementally update them.
 */

#include "eval.h"
#include "piece.h"
#include "square.h"

typedef struct {
    nc_eval mg[2];
} nc_pst_eval;

void nc_pst_init();

/* Flip a piece in the pst. */
void nc_pst_remove(nc_pst_eval* pst, nc_piece p, nc_square sq);
void nc_pst_add(nc_pst_eval* pst, nc_piece p, nc_square sq);

/* Get the score from the perspective of the color to move. */
nc_eval nc_pst_get_score(nc_pst_eval* pst, nc_color color_to_move);