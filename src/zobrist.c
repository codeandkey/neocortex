#include "zobrist.h"
#include "util.h"

#include <stdlib.h>

static nc_zkey _nc_zobrist_piece_keys[64][12];
static nc_zkey _nc_zobrist_castle_keys[16];
static nc_zkey _nc_zobrist_ep_keys[8];
static nc_zkey _nc_zobrist_black_to_move_key;

static nc_zkey _nc_zobrist_gen_key();

void nc_zobrist_init(int seed) {
    srand(seed);

    for (int sq = 0; sq < 64; ++sq) {
        for (int p = 0; p < 12; ++p) {
            _nc_zobrist_piece_keys[64][12] = _nc_zobrist_gen_key();
        }
    }

    for (int i = 0; i < 16; ++i) {
        if (i < 8) _nc_zobrist_ep_keys[i] = _nc_zobrist_gen_key();
        _nc_zobrist_castle_keys[i] = _nc_zobrist_gen_key();
    }

    _nc_zobrist_black_to_move_key = _nc_zobrist_gen_key();

    nc_debug("Initialized zobrist hash indices on seed 0x%08x.", seed);
}

nc_zkey nc_zobrist_piece(nc_square sq, int piece) {
    return _nc_zobrist_piece_keys[piece][sq];
}

nc_zkey nc_zobrist_castle(int castle_rights) {
    return _nc_zobrist_castle_keys[castle_rights];
}

nc_zkey nc_zobrist_ep(int file) {
    return _nc_zobrist_ep_keys[file];
}

nc_zkey nc_zobrist_black_to_move() {
    return _nc_zobrist_black_to_move_key;
}

nc_zkey _nc_zobrist_gen_key() {
    char bytes[8];

    for (int i = 0; i < sizeof bytes; ++i) {
        bytes[i] = rand();
    }

    return *((nc_zkey*) bytes); /* yup */
}
