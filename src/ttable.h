#pragma once

#include "position.h"
#include "result.h"

namespace nc2 {
    namespace ttable {
        static constexpr int TTABLE_WIDTH = 65536;

        /**
         * Looks up an entry in the transposition table.
         *
         * @param key Transposition key to look up.
         *
         * @return Saved position result, or NULL if not found.
         */
        Result* lookup(u64 key);

        /**
         * Stores a new result in the transposition table.
         *
         * @param key Transposition key to store under.
         * @param res Search result for position.
         */
        void store(Result& res);
    }
}
