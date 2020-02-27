#pragma once

#include "square.h"
#include "bb.h"

void nc_magic_init();
void nc_magic_free();

nc_bb nc_magic_query_rook_attacks(nc_square sq, nc_bb occ);
nc_bb nc_magic_query_bishop_attacks(nc_square sq, nc_bb occ);
