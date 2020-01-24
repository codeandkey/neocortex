#include "log.h"
#include "lookup.h"
#include "bitboard.h"
#include "position.h"
#include "move.h"
#include "game.h"
#include "engine.h"

#include <iostream>

int main(int argc, char** argv) {
    nc::log_init();
    nc_info("Started neocortex.");

    nc::lookup::init();
    nc_info("Built attack lookups.");

    /*nc::Game g;
    g.apply(std::string("e2e4"));
    g.apply(std::string("g8f6"));
    g.apply(std::string("b1c3"));
    g.apply(std::string("e7e5"));
    g.apply(std::string("d2d3"));
    g.apply(std::string("f8b4"));
    g.apply(std::string("a2a3"));
    g.apply(std::string("b4c3"));
    g.apply(std::string("b2c3"));

    nc::Position p = g.get_current_position();

    std::list<nc::Position::Transition> l = p.get_legal_moves();

    for (auto i : l) {
        std::cout << "legal move: " << i.to_string() << "\n";
    }

    return 0;*/

    /* Start a uci stream. */
    nc::Engine eng(std::cin, std::cout);
    eng.run_uci_interface();


    /*
    nc::Game g;

    std::cout << "Started a game. Enter in some moves:\n";
    while (1) {
        auto x = g.get_legal_next_moves();

        if (x.size() < 4) {
            std::cout << "[";
            for (auto m : x) {
                std::cout << " " << m.to_string() << " ";
            }
            std::cout << "]";
        } else {
            std::cout << "(" << std::to_string(x.size()) << " legal moves)";
        }

        std::cout << "> ";
        std::string inp;

        std::cin >> inp;

        try {
            nc::Position::Transition t = g.apply(inp);

            std::cout << "Applied matched move " << t.to_string() << "\n";
            std::cout << "New game FEN: " << g.get_current_position().get_fen() << "\n";
        } catch (std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << "\n";
        }
    }
    */

    return 0;
}
