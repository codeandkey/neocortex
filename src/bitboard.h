#pragma once

/**
 * Utilities for manipulating bitboards.
 */

#include "types.h"

#include <string>

namespace nc {
    namespace bitboard {
        /**
         * Converts a bitboard to a pretty-printed string.
         *
         * @param b Board to convert
         * @return Printable bitboard string representation.
         */
        std::string to_string(u64 b);
    }
}
