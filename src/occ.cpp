#include "occ.h"
#include "square.h"

#include <cassert>
#include <cstring>

using namespace nc2;

static u64 _nc2_occboard_pawn_jump_masks[8][2] = { /* [file][color] */
    { square::MASK_A3 | square::MASK_A4, square::MASK_A6 | square::MASK_A5 },
    { square::MASK_B3 | square::MASK_B4, square::MASK_B6 | square::MASK_B5 },
    { square::MASK_C3 | square::MASK_C4, square::MASK_C6 | square::MASK_C5 },
    { square::MASK_D3 | square::MASK_D4, square::MASK_D6 | square::MASK_D5 },
    { square::MASK_E3 | square::MASK_E4, square::MASK_E6 | square::MASK_E5 },
    { square::MASK_F3 | square::MASK_F4, square::MASK_F6 | square::MASK_F5 },
    { square::MASK_G3 | square::MASK_G4, square::MASK_G6 | square::MASK_G5 },
    { square::MASK_H3 | square::MASK_H4, square::MASK_H6 | square::MASK_H5 },
};

static u64 _nc2_occboard_castle_masks[2][2] = { /* [color][kingside] */
    { square::MASK_B1 | square::MASK_C1 | square::MASK_D1, square::MASK_F1 | square::MASK_G1 },
    { square::MASK_B8 | square::MASK_C8 | square::MASK_D8, square::MASK_F8 | square::MASK_G8 },
};

Occboard::Occboard() {
    memset(this, 0, sizeof *this);
}

Occboard Occboard::standard() {
    Occboard out;

    for (int f = 0; f < 8; ++f) {
        out.flip(square::at(0, f));
        out.flip(square::at(1, f));
        out.flip(square::at(6, f));
        out.flip(square::at(7, f));
    }

    return out;
}

void Occboard::flip(u8 s) {
    u8 r = square::rank(s), f = square::file(s);
    u8 rb = (1 << r);

    diags[square::diag(s)] ^= rb;
    antidiags[square::antidiag(s)] ^= rb;
    ranks[r] ^= (1 << f);
    files[f] ^= rb;

    board ^= square::mask(s);
}

u64 Occboard::get_board() {
    return board;
}

u8 Occboard::get_rank(u8 r) {
    assert(r < 8);
    return ranks[r];
}

u8 Occboard::get_file(u8 f) {
    assert(f < 8);
    return files[f];
}

u8 Occboard::get_diag(u8 d) {
    assert(d < 15);
    return diags[d];
}

u8 Occboard::get_antidiag(u8 d) {
    assert(d < 15);
    return antidiags[d];
}

bool Occboard::pawn_can_jump(u8 f, u8 col) {
    return !test_mask(_nc2_occboard_pawn_jump_masks[f][col]);
}

bool Occboard::color_can_castle(u8 col, int kingside) {
    return !test_mask(_nc2_occboard_castle_masks[col][kingside]);
}

bool Occboard::test(u8 s) {
    return (board & square::mask(s));
}

bool Occboard::test_mask(u64 m) {
    return (board & m);
}

u8 Occboard::to_rocc(u8 occ_byte) {
    return (occ_byte >> 1) & 63;
}
