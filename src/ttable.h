#pragma once

#include "position.h"
#include "eval_type.h"

namespace nc2 {
    namespace ttable {
        static constexpr int TTABLE_WIDTH = 16384;

        bool lookup(Position* p, Evaluation* saved_eval, int mindepth);
        void store(Position* p, Evaluation position_eval, int depth);

        struct Entry {
            Entry(Position p, Evaluation e, int d);

            Position p;
            Evaluation e;
            int depth;
        };
    }
}
