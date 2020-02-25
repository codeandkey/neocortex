#pragma once

/*
 * Zobrist key management.
 */

#include <stdint.h>

#include "square.h"

typedef uint64_t nc_zkey;

void nc_zobrist_init(int seed);

nc_zkey nc3_zobrist_piece(nc_square sq, int piece);
nc_zkey nc3_zobrist_castle(int castle_rights); /* 16 keys because it doesn't matter */
nc_zkey nc3_zobirst_ep(int file);
nc_zkey nc3_zobrist_black_to_move();
