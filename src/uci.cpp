#include "uci.h"
#include "log.h"
#include "movegen.h"
#include "util.h"
#include "search.h"
#include "position.h"
#include "options.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace pine;

static std::string read_command(std::istream& in) {
	std::string output;
	std::getline(in, output);

	return util::trim(output);
}

void uci::connect(std::istream& in, std::ostream& out) {
	std::string handshake  = read_command(in);

	if (handshake != "uci") {
		throw std::runtime_error(util::format("Invalid UCI: expected 'uci', read '%s'", handshake.c_str()));
	}

	out << "uciok\n";
	out << "id name " << uci::NAME << "\n";
	out << "id author " << uci::AUTHOR << "\n";
}

void uci::begin(std::istream & in, std::ostream & out) {
	search::Search searcher;

	while (true) {
		std::string command = read_command(in);
		std::vector<std::string> parts = util::split(command, ' ');

		if (!parts.size()) continue;
			
		if (parts[0] == "quit") {
			break;
		}
		else if (parts[0] == "isready") {
			out << "readyok\n";
		}
		else if (parts[0] == "debug") {
			if (parts.size() < 2) {
				pine_warn("Invalid UCI: debug expects an argument (\"on\" or \"off\")\n");
				continue;
			}

			if (parts[1] == "on") {
				searcher.set_debug(true);
			}
			else if (parts[1] == "off")
			{
				searcher.set_debug(false);
			}
			else {
				pine_warn("Invalid UCI: debug expects an argument (\"on\" or \"off\")\n");
			}
		}
		else if (parts[0] == "stop") {
			searcher.stop();
		}
		else if (parts[0] == "go") {
			/* Parse arguments */
			searcher.clear_go_params();

			std::string ucierr = "";
			try {
				for (size_t i = 1; i < parts.size(); ++i) {
					if (parts[i] == "wtime") {
						searcher.set_wtime(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "btime") {
						searcher.set_btime(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "winc") {
						searcher.set_winc(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "binc") {
						searcher.set_binc(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "winc") {
						searcher.set_winc(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "depth") {
						searcher.set_depth(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "nodes") {
						searcher.set_nodes(std::stoi(parts[i + 1]));
					}

					if (parts[i] == "movetime") {
						searcher.set_movetime(std::stoi(parts[i + 1]));
					}
				}
			}
			catch (std::exception & e) {
				pine_warn("Invalid UCI in command: caught %s\n", e.what());
				continue;
			}

			searcher.go(out);
		}
		else if (parts[0] == "position") {
			searcher.stop();

			if (parts.size() < 2) {
				pine_warn("Invalid UCI: position must be followed by 'startpos' or a FEN\n");
				continue;
			}

			Position pos;
			int expected_moves = 2;

			if (parts[1] != "startpos") {
				if (parts.size() < 7) {
					pine_warn("Invalid UCI: not enough FEN fields for position\n");
					continue;
				}

				std::string fen = util::join(std::vector<std::string>(parts.begin() + 1, parts.begin() + 7), " ");

				pos = fen;
				expected_moves = 6;
			}

			if (expected_moves < (int) parts.size()) {
				if (parts[expected_moves] != "moves") {
					pine_warn("Invalid UCI: expected 'moves', read '%s'\n", parts[expected_moves].c_str());
					continue;
				}

				for (size_t i = expected_moves + 1; i < parts.size(); ++i) {
					movegen::Generator g(pos);
					Move matched_move;

					for (auto movelist : g.generate_perft()) {
						if (!movelist.size()) continue;
						for (auto move : movelist) {
							if (move.match_uci(parts[i])) {
								assert(matched_move == Move::null);
								matched_move = move;
							}
						}
					}

					if (matched_move == Move::null) {
						pine_warn("Invalid UCI: unmatched move '%s'\n", parts[i].c_str());
						break;
					}

					pos.make_move(matched_move);
				}
			}

			searcher.load(pos);
			pine_debug("Loaded position %s\n", pos.to_fen().c_str());
		}
	}
}
