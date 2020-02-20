#pragma once

#include "types.h"

void nc3_magic_init();
void nc3_magic_free();

const nc3_movelist* nc3_magic_query_rook_moves(nc3_square sq, u64 occ);
const nc3_movelist* nc3_magic_query_bishop_moves(nc3_square sq, u64 occ);
u64 nc3_magic_query_rook_attacks(nc3_square sq, u64 occ);
u64 nc3_magic_query_bishop_attacks(nc3_square sq, u64 occ);
