#pragma once

#include "position.h"
#include "result.h"

namespace nc2 {
    namespace ttable {
        static constexpr int TTABLE_WIDTH = 65536;

        /**
         * Looks up an entry in the transposition table.
         *
         * @param p Position to look up.
         *
         * @return Saved position result, or NULL if not found.
         */
        search::Result* lookup(Position* p);

        /**
         * Stores a new result in the transposition table.
         *
         * @param p Position to store.
         * @param res Search result for position.
         */
        void store(Position* p, search::Result res);
    }
}
