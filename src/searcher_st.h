#pragma once

#include <iostream>
#include <vector>
#include <list>

#include "position.h"
#include "eval_type.h"
#include "move.h"
#include "result.h"

namespace nc2 {
    class SearcherST {
        public:
            SearcherST(std::ostream& uci_out);

            void go(int wtime, int btime);
            void set_position(Position p);

        private:
            std::ostream& uci_out;
            Position root;

            typedef std::pair<Move, search::Result> Edge;

            search::Result alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta);
            search::Result quiescence(Position* p, Evaluation alpha, Evaluation beta);

            int nodes;
            int thits;
    };
}
