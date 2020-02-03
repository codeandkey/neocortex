#pragma once

/*
 * Square utilities
 */

#include <string>

#include "types.h"

namespace nc2 {
    namespace square {
        constexpr u8 null = 0xFF;

        void init_lookups();

        u8 at(u8 r, u8 f);
        u8 at_safe(u8 r, u8 f);

        u8 rank(u8 s);
        u8 file(u8 s);
        u8 shift(u8 s, int r, int f);

        u8 diag(u8 s);
        u8 antidiag(u8 s);

        u64 mask(u8 s);

        std::string to_string(u8 s);

        enum Squares {
            a1, b1, c1, d1, e1, f1, g1, h1,
            a2, b2, c2, d2, e2, f2, g2, h2,
            a3, b3, c3, d3, e3, f3, g3, h3,
            a4, b4, c4, d4, e4, f4, g4, h4,
            a5, b5, c5, d5, e5, f5, g5, h5,
            a6, b6, c6, d6, e6, f6, g6, h6,
            a7, b7, c7, d7, e7, f7, g7, h7,
            a8, b8, c8, d8, e8, f8, g8, h8,
        };
    }
}
