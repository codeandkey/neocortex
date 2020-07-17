#pragma once

#include "position.h"

typedef struct {
    nc_eval king_safety[2];
    nc_eval material_nonpawn_mg[2];
    nc_eval material_nonpawn_eg[2];
    nc_eval material_pawn_mg[2];
    nc_eval material_pawn_eg[2];
    nc_eval center_control[2];
    nc_eval development[2];
    nc_eval phase; /* 0 through 255 inclusive where 255 => opening, 0 => endgame */
    nc_eval score;
} nc_position_eval;

void nc_eval_position(nc_position* p, nc_position_eval* dst);
