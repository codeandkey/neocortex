#pragma once

#include <vector>

#include "move.h"
#include "types.h"
#include "occ.h"

namespace nc2 {
    namespace lookup {
        void initialize_rook_lookup();

        const std::vector<Move>& rook_moves(u8 s, Occboard* occ);
        u64 rook_attacks(u8 s, Occboard* occ);
    }
}
