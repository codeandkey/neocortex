#include "tindex.h"

#include <cmath>

using namespace nc2;

static u32 _nc2_tindex_piece_keys[6][2][64];
static u32 _nc2_tindex_castle_keys[2][2];
static u32 _nc2_tindex_black_to_move_key;
static u32 _nc2_tindex_ep_keys[8];

void ttable::initialize_indices(u32 seed) {
    srand(seed);

    for (int t = 0; t < 6; ++t) {
        for (int c = 0; c < 2; ++c) {
            for (int s = 0; s < 64; ++s) {
                _nc2_tindex_piece_keys[t][c][s] = rand();
            }
        }
    }

    for (int c = 0; c < 2; ++c) {
        for (int s = 0; s < 2; ++s) {
            _nc2_tindex_castle_keys[c][s] = rand();
        }
    }

    _nc2_tindex_black_to_move_key = rand();

    for (int f = 0; f < 8; ++f) {
        _nc2_tindex_ep_keys[f] = rand();
    }
}

u32 ttable::get_piece_key(u8 s, u8 type, u8 col) {
    return _nc2_tindex_piece_keys[type][col][s];
}

u32 ttable::get_castle_key(u8 c, u8 side) {
    return _nc2_tindex_castle_keys[c][side];
}

u32 ttable::get_black_to_move_key() {
    return _nc2_tindex_black_to_move_key;
}

u32 ttable::get_en_passant_file_key(u8 f) {
    return _nc2_tindex_ep_keys[f];
}
