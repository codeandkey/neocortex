#pragma once

/*
 * Zobrist key management.
 */

#include "types.h"

void nc3_zobrist_init(int seed);

u64 nc3_zobrist_piece(nc3_square sq, int piece);
u64 nc3_zobrist_castle(int castle_rights); /* 16 keys because it doesn't matter */
u64 nc3_zobirst_ep(int file);
u64 nc3_zobrist_black_to_move();
