#include "zobrist.h"

#include <stdlib.h>

static u64 _nc3_zobrist_piece_keys[64][12];
static u64 _nc3_zobrist_castle_keys[16];
static u64 _nc3_zobrist_ep_keys[8];
static u64 _nc3_zobrist_black_to_move_key;

static u64 _nc3_zobrist_gen_key();

void nc3_zobrist_init(int seed) {
    srand(seed);

    for (int sq = 0; sq < 64; ++sq) {
        for (int p = 0; p < 12; ++p) {
            _nc3_zobrist_piece_keys[64][12] = _nc3_zobrist_gen_key();
        }
    }

    for (int i = 0; i < 16; ++i) {
        if (i < 8) _nc3_zobrist_ep_keys[i] = _nc3_zobrist_gen_key();
        _nc3_zobrist_castle_keys[i] = _nc3_zobrist_gen_key();
    }

    _nc3_zobrist_black_to_move_key = _nc3_zobrist_gen_key();
}

u64 nc3_zobrist_piece(nc3_square sq, int piece) {
    return _nc3_zobrist_piece_keys[piece][sq];
}

u64 nc3_zobrist_castle(int castle_rights) {
    return _nc3_zobrist_castle_keys[castle_rights];
}

u64 nc3_zobrist_ep(int file) {
    return _nc3_zobrist_ep_keys[file];
}

u64 nc3_zobrist_black_to_move() {
    return _nc3_zobrist_black_to_move_key;
}

u64 _nc3_zobrist_gen_key() {
    char bytes[8];

    for (int i = 0; i < sizeof bytes; ++i) {
        bytes[i] = rand();
    }

    return *((u64*) bytes); /* yup */
}
