#pragma once

#include <iostream>

#include "position.h"
#include "eval_type.h"

namespace nc2 {
    class SearcherST {
        public:
            SearcherST(std::ostream& uci_out);

            void go();
            void set_position(Position p);

        private:
            std::ostream& uci_out;
            Position root;

            Evaluation alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta, Move* bestmove_out);
            Evaluation quiescence(Position* p, int d, Evaluation alpha, Evaluation beta);

            int nodes;
            int thits;

            static constexpr int DEPTH = 5;
            static constexpr int QDEPTH = 5;
    };
}
