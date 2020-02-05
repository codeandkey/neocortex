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

        const std::vector<Move>& pawn_advances(u8 s, u8 col);
        const std::vector<Move>& pawn_captures(u8 s, u8 col);

        u64 pawn_attacks(u8 s, u8 col);
    }
}
