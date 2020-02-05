#pragma once

#include <iostream>

#include "position.h"
#include "game.h"
#include "searcher_st.h"

namespace nc2 {
    class Engine {
        public:
            Engine(std::ostream& uci_out, std::istream& uci_in);

            void start_uci();
        private:
            std::istream& uci_in;
            std::ostream& uci_out;
            SearcherST searcher;

            Game current_game;
    };
}
