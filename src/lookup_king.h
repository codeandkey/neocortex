#pragma once

#include <vector>

#include "move.h"
#include "types.h"

namespace nc2 {
    namespace lookup {
        void initialize_king_lookup();

        std::vector<Move> king_moves(u8 s);
    }
}
