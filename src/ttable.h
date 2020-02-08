#pragma once

#include "position.h"
#include "result.h"

namespace nc2 {
    namespace ttable {
        static constexpr int TTABLE_WIDTH = 65536;

        search::Result* lookup(Position* p);
        void store(Position* p, search::Result res);
    }
}
