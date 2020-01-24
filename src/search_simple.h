#pragma once

/*
 * Simple single-threaded searcher.
 */

#include "search.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <future>

namespace nc {
    class SearchSimple : public Search {
        public:
            SearchSimple(std::ostream& uci_out);

            void set_position(Position p);
            void go();
            void stop();
            void set_depth(int depth);

        private:
            int depth;
            std::thread search_worker;
            Position root_position;

            std::atomic<bool> search_running;

            void worker_func();
            Evaluation alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta, Move* bestmove);
    };
}
