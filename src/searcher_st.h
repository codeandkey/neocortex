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
            /**
             * Initializes a new Searcher.
             *
             * @param uci_out UCI output stream.
             */
            SearcherST(std::ostream& uci_out);

            /**
             * Starts a synchronous search on the current position.
             *
             * @param wtime White time (ms)
             * @param btime Black time (ms)
             */

            void go(int wtime, int btime);

            /**
             * Sets the current root position to search from.
             *
             * @param p New root position.
             */
            void set_position(Position p);

            /**
             * Runs a synchronous search at a certain depth.
             *
             * @param start Starting position.
             * @param d Search depth.
             *
             * @return Search result.
             */
            search::Result run_search(Position* start, int depth);

        private:
            std::ostream& uci_out;
            Position root;

            typedef std::pair<Move, search::Result> Edge;

            /**
             * Runs an alpha beta search on a position.
             *
             * @param p Starting position
             * @param d Search depth
             * @param alpha Current alpha
             * @param beta Current beta
             *
             * @return Search result.
             */
            search::Result alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta);

            /**
             * Runs a quiescence search on a non-quiet position.
             *
             * @param p Starting position
             * @param alpha Current alpha
             * @param beta Current beta
             *
             * @return Search result (will be of 0 depth)
             */
            search::Result quiescence(Position* p, Evaluation alpha, Evaluation beta);

            int nodes;
            int thits;
    };
}
