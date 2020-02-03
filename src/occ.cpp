#include "occ.h"
#include "square.h"

#include <cassert>
#include <cstring>

using namespace nc2;

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

    board ^= (1 << s);
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

bool Occboard::test(u8 s) {
    return (board & square::mask(s));
}

u8 Occboard::to_rocc(u8 occ_byte) {
    return (occ_byte >> 1) & 63;
}
