#pragma once

#include <iostream>

#include "position.h"

namespace nc2 {
    class SearcherST {
        public:
            SearcherST(std::ostream& uci_out);

            void go();
            void set_position(Position p);

        private:
            std::ostream& uci_out;
            Position root;
    };
}
