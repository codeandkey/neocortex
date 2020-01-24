#pragma once

/**
 * Searcher interface.
 */

#include <iostream>

#include "position.h"
#include "eval.h"
#include "move.h"

namespace nc {
    class Search {
        public:
            Search(std::ostream& uci_out);

            virtual ~Search();

            virtual void set_position(Position p) = 0;
            virtual void go() = 0;
            virtual void stop() = 0;

        protected:
            void write_info(int depth, Evaluation eval, Move* currmove);
            void write_bestmove(Move bestmove);

        private:
            std::ostream& uci_out;
    };
}
