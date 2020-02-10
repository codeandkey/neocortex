#include "square.h"

#include <cassert>

using namespace nc2;

static u8 _nc2_square_diag_lookup[64] = {
    7, 6, 5, 4, 3, 2, 1, 0,
    8, 7, 6, 5, 4, 3, 2, 1,
    9, 8, 7, 6, 5, 4, 3, 2,
    10, 9, 8, 7, 6, 5, 4, 3,
    11, 10, 9, 8, 7, 6, 5, 4,
    12, 11, 10, 9, 8, 7, 6, 5,
    13, 12, 11, 10, 9, 8, 7, 6,
    14, 13, 12, 11, 10, 9, 8, 7,
}, _nc2_square_antidiag_lookup[64] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    1, 2, 3, 4, 5, 6, 7, 8,
    2, 3, 4, 5, 6, 7, 8, 9,
    3, 4, 5, 6, 7, 8, 9, 10,
    4, 5, 6, 7, 8, 9, 10, 11,
    5, 6, 7, 8, 9, 10, 11, 12,
    6, 7, 8, 9, 10, 11, 12, 13,
    7, 8, 9, 10, 11, 12, 13, 14,
};

u8 square::at(u8 r, u8 f) {
    assert(r < 8 && f < 8);
    return r * 8 + f;
}

u8 square::at_safe(u8 r, u8 f) {
    if (r >= 8 || f >= 8) return square::null;
    return r * 8 + f;
}

u8 square::rank(u8 s) {
    assert(s < 64);
    return s >> 3;
}

u8 square::file(u8 s) {
    assert(s < 64);
    return s & 7;
}

u8 square::diag(u8 s) {
    assert(s < 64);
    return _nc2_square_diag_lookup[s];
}

u8 square::antidiag(u8 s) {
    assert(s < 64);
    return _nc2_square_antidiag_lookup[s];
}

u8 square::shift(u8 s, int r, int f) {
    assert(square::rank(s) + r >= 0);
    assert(square::rank(s) + r < 8);
    assert(square::file(s) + f >= 0);
    assert(square::file(s) + f < 8);

    return s + 8 * r + f;
}


u64 square::mask(u8 s) {
    return (u64) 1 << s;
}

std::string square::to_string(u8 s) {
    if (s == square::null) return "-";

    assert(s < 64);

    u8 r = square::rank(s), f = square::file(s);

    std::string out;

    out += f + 'a';
    out += r + '1';

    return out;
}
