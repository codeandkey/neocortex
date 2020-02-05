#pragma once

#include <vector>

#include "move.h"
#include "types.h"

namespace nc2 {
    namespace lookup {
        void initialize_knight_lookup();

        const std::vector<Move>& knight_moves(u8 s);
        u64 knight_attacks(u8 s);
    }
}
