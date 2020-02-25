#pragma once

/*
 * Basic lookups implement simple lookup tables for knights and kings.
 */

#include "bb.h"
#include "square.h"

void nc_basic_init();

nc_bb nc_basic_knight_attacks(nc_square src);
nc_bb nc_basic_king_attacks(nc_square src);
