#pragma once

/**
 * Bitboard structure.
 *
 * This contains the class for storing bitboards along with a ton of functions for
 * taking advantage of them for generating attacked squares. This is also particularly useful for
 * generating valid moves by walking through the bits of an attack bitboard.
 */

#include <stdint.h>

namespace nc {
    class Bitboard {
    public:
        Bitboard();

        void set(Square s);
        void unset(Square s);
    };
}

