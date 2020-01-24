#include "engine.h"
#include "version.h"
#include "log.h"

#include "search_simple.h"

#include <sstream>

using namespace nc;

Engine::Engine(std::istream& uci_in, std::ostream& uci_out) : uci_input(uci_in), uci_output(uci_out) {
    searcher = new SearchSimple(uci_out);
}

Engine::~Engine() {
    delete searcher;
}

void Engine::run_uci_interface() {
    std::string command;

    /* Disable buffered output. */
    uci_output.setf(std::ios::unitbuf);

    try {
        std::getline(uci_input, command);

        if (command != "uci") {
            throw std::runtime_error(std::string("invalid uci: expected 'uci', read '") + command + ")");
        }

        uci_output << "id name " << Version::NAME << " " << std::to_string(Version::MAJOR) << "." << std::to_string(Version::MINOR) << "\n";
        uci_output << "id author " << Version::AUTHOR << "\n";
        uci_output << "uciok\n";

        while (1) {
            std::getline(uci_input, command);

            if (command == "quit") {
                nc_info("Received quit.");
                break;
            }

            if (command == "isready") {
                uci_output << "readyok\n";
                continue;
            }

            if (command == "stop") {
                searcher->stop();
                continue;
            }

            /* Break command into words. */
            std::stringstream words;
            words << command;

            std::string word;
            words >> word;

            if (word == "go") {
                searcher->go();
            }

            if (word == "position") {
                words >> word;

                if (word == "startpos") {
                    current_game = Game();
                } else {
                    throw std::runtime_error("FIXME uci error: expected 'startpos'");
                }

                if (words >> word) {
                    if (word == "moves") {
                        while (words >> word) {
                            current_game.apply(word);
                        }
                    } else {
                        throw std::runtime_error("uci error: expected 'moves'");
                    }
                }

                searcher->set_position(current_game.get_current_position());
            } else {
                nc_warning("Unrecognized command %s, ignoring", word.c_str());
            }
        }
    } catch (std::exception& e) {
        nc_error("Engine runtime exception: %s", e.what());
    }

    nc_info("Engine terminating cleanly.");
}

void Engine::set_game(const Game& g) {
    current_game = g;
}

void Engine::go() {
    searcher->go();
}
