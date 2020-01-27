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
            ~SearchSimple();

            void set_position(Position p);
            void go();
            void stop();
            void set_depth(int depth);

        private:
            int depth, quiescence_depth;
            const float eval_noise = 0.001;
            std::thread search_worker;
            Position root_position;

            std::atomic<bool> search_running;

            void worker_func();
            Evaluation alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta, Move* bestmove, bool top);
            Evaluation quiescence_search(Position* p, int d, Evaluation alpha, Evaluation beta);
    };
}
