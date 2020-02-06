#include "engine.h"
#include "version.h"

#include <sstream>

using namespace nc2;

Engine::Engine(std::ostream& uci_out, std::istream& uci_in) : uci_in(uci_in), uci_out(uci_out), searcher(uci_out) {}

void Engine::start_uci() {
    std::string command;

    /* Disable buffered output. */
    uci_out.setf(std::ios::unitbuf);

    try {
        std::getline(uci_in, command);

        if (command != "uci") {
            throw std::runtime_error(std::string("invalid uci: expected 'uci', read '") + command + ")");
        }

        uci_out << "id name " << version::NAME << " " << std::to_string(version::MAJOR) << "." << std::to_string(version::MINOR) << "\n";
        uci_out << "id author " << version::AUTHOR << "\n";
        uci_out << "uciok\n";

        while (1) {
            std::getline(uci_in, command);

            if (command == "quit") {
                break;
            }

            if (command == "isready") {
                uci_out << "readyok\n";
                continue;
            }

            if (command == "stop") {
                //searcher.stop();
                continue;
            }

            /* Break command into words. */
            std::stringstream words;
            words << command;

            std::string word;
            words >> word;

            if (word == "go") {
                int wtime = -1, btime = -1;

                while (words >> word) {
                    if (word == "wtime") {
                        words >> wtime;
                    } else if (word == "btime") {
                        words >> btime;
                    }
                }

                searcher.go(wtime, btime);
                continue;
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

                searcher.set_position(current_game.get_current_position());
            } else {
                std::cerr << "Unrecognized command " << word << ", ignoring\n";
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Engine runtime exception: " << e.what() << "\n";
    }
}
