#pragma once

#include <iostream>
#include <string>

#include "game.h"
#include "search.h"

namespace nc {
    class Engine {
        public:
            /**
             * Initializes a UCI engine.
             * The engine controls control input, control output, and work dispatch.
             *
             * @param uci_input UCI input stream
             * @param uci_output UCI output stream
             */
            Engine(std::istream& uci_in, std::ostream& uci_out);

            ~Engine();

            /**
             * Accepts UCI input from the input stream.
             * Starts waiting for commands until the engine is stopped via
             * UCI or signal.
             */
            void run_uci_interface();

            /**
             * Sets the current engine game;
             *
             * @param g Game to assign.
             */
            void set_game(const Game& g);

            /**
             * Starts a search on a position.
             */
            void go();

            /**
             * Stops any running searches.
             * Synchronous.
             */
            void stop();

        private:
            std::istream& uci_input;
            std::ostream& uci_output;

            Game current_game;
            Search* searcher;
    };
}
