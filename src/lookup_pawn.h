#pragma once

/*
 * Pawn movegen functions.
 */

#include <vector>
#include <iterator>

#include "move.h"

namespace nc2 {
    namespace lookup {
        void initialize_pawn_lookup();

        std::vector<Move>::iterator pawn_advances(u8 s, u8 col);
        std::vector<Move>::iterator pawn_captures(u8 s, u8 col);
    }
}
