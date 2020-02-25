#pragma once

/*
 * Zobrist key management.
 */

#include <stdint.h>

#include "square.h"
#include "piece.h"

typedef uint64_t nc_zkey;

void nc_zobrist_init(int seed);

nc_zkey nc_zobrist_piece(nc_square sq, nc_piece piece);
nc_zkey nc_zobrist_castle(int castle_rights); /* 16 keys because it doesn't matter */
nc_zkey nc_zobirst_ep(int file);
nc_zkey nc_zobrist_black_to_move();
