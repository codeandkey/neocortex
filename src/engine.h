#pragma once

#include <iostream>

#include "position.h"
#include "game.h"
#include "searcher_st.h"

namespace nc2 {
    class Engine {
        public:
            /**
             * Initializes a new Engine.
             *
             * @param uci_out UCI output stream.
             * @param uci_in  UCI input stream.
             */
            Engine(std::ostream& uci_out, std::istream& uci_in);

            /**
             * Starts a UCI interface on the initial streams.
             * Returns once the interface is closed or shut down.
             */
            void start_uci();
        private:
            std::istream& uci_in;
            std::ostream& uci_out;
            SearcherST searcher;

            Game current_game;
    };
}
