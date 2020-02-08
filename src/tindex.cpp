#include "tindex.h"

#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <limits>

using namespace nc2;

static u64 _nc2_tindex_piece_keys[16][64];
static u64 _nc2_tindex_castle_keys[2][2];
static u64 _nc2_tindex_black_to_move_key;
static u64 _nc2_tindex_ep_keys[8];

void ttable::initialize_indices(u32 seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<u64> dist;

    for (int p = 0; p < 0xF; ++p) {
        for (int s = 0; s < 64; ++s) {
            _nc2_tindex_piece_keys[p][s] = dist(rng);
        }
    }

    for (int c = 0; c < 2; ++c) {
        for (int s = 0; s < 2; ++s) {
            _nc2_tindex_castle_keys[c][s] = dist(rng);
        }
    }

    _nc2_tindex_black_to_move_key = dist(rng);

    for (int f = 0; f < 8; ++f) {
        _nc2_tindex_ep_keys[f] = dist(rng);
    }

    srand(time(NULL));
}

u64 ttable::get_piece_key(u8 s, u8 p) {
    return _nc2_tindex_piece_keys[p][s];
}

u64 ttable::get_castle_key(u8 c, u8 side) {
    return _nc2_tindex_castle_keys[c][side];
}

u64 ttable::get_black_to_move_key() {
    return _nc2_tindex_black_to_move_key;
}

u64 ttable::get_en_passant_file_key(u8 f) {
    return _nc2_tindex_ep_keys[f];
}
