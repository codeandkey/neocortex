#pragma once

/*
 * Piece constants and helpers.
 */

namespace nc2 {
    namespace piece {
        enum Type {
            PAWN,
            KNIGHT,
            BISHOP,
            ROOK,
            KING,
            QUEEN,
            NONE,
        };

        enum Color {
            WHITE,
            BLACK,
        };
    }
}
