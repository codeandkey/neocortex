#pragma once

#include <vector>

#include "move.h"
#include "types.h"
#include "occ.h"

namespace nc2 {
    namespace lookup {
        void initialize_bishop_lookup();

        const std::vector<Move>& bishop_moves(u8 s, Occboard* occ);
        u64 bishop_attacks(u8 s, Occboard* occ);
    }
}
