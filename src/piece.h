#pragma once

/*
 * Piece constants and helpers.
 */

#include "types.h"

namespace nc2 {
    namespace piece {
        enum Type {
            PAWN   = 0,
            KNIGHT = 1,
            BISHOP = 2,
            ROOK   = 3,
            KING   = 4,
            QUEEN  = 5,
            NONE   = 7,
        };

        enum Color {
            WHITE = 0,
            BLACK = 1,
        };

        constexpr u8 null = 0xF;

        constexpr u8 WHITE_PAWN = 0;
        constexpr u8 BLACK_PAWN = 1;
        constexpr u8 WHITE_KNIGHT= 2;
        constexpr u8 BLACK_KNIGHT = 3;
        constexpr u8 WHITE_BISHOP = 4;
        constexpr u8 BLACK_BISHOP = 5;
        constexpr u8 WHITE_ROOK = 6;
        constexpr u8 BLACK_ROOK = 7;
        constexpr u8 WHITE_KING = 8;
        constexpr u8 BLACK_KING = 9;
        constexpr u8 WHITE_QUEEN = 10;
        constexpr u8 BLACK_QUEEN = 11;

        u8 make(u8 type, u8 color);
        u8 type(u8 p);
        u8 color(u8 p);

        char uci(u8 p);
        char type_char(u8 t);
        bool exists(u8 p);
    }
}
