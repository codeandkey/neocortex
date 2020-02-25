#pragma once

#include "square.h"
#include "bb.h"

void nc_magic_init();
void nc_magic_free();

nc_bb_t nc_magic_query_rook_attacks(nc_square_t sq, nc_bb_t occ);
nc_bb_t nc_magic_query_bishop_attacks(nc_square_t sq, nc_bb_t occ);
