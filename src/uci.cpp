/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "uci.h"
#include "log.h"
#include "movegen.h"
#include "util.h"
#include "search.h"
#include "position.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace neocortex;

static std::string read_command(std::istream& in) {
	std::string output;
	std::getline(in, output);

	return util::trim(output);
}

void uci::start(std::istream & in, std::ostream & out) {
	std::string handshake  = read_command(in);

	if (handshake != "uci") {
		throw std::runtime_error(util::format("Invalid UCI: expected 'uci', read '%s'", handshake.c_str()));
	}

	out << "uciok\n";
	out << "id name " << uci::NAME << "\n";
	out << "id author " << uci::AUTHOR << "\n";

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
				neocortex_warn("Invalid UCI: debug expects an argument (\"on\" or \"off\")\n");
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
				neocortex_warn("Invalid UCI: debug expects an argument (\"on\" or \"off\")\n");
			}
		}
		else if (parts[0] == "stop") {
			searcher.stop();
		}
		else if (parts[0] == "go") {
			searcher.go(parts, out);
		}
		else if (parts[0] == "position") {
			searcher.stop();

			if (parts.size() < 2) {
				neocortex_warn("Invalid UCI: position must be followed by 'startpos' or a FEN\n");
				continue;
			}

			Position pos;
			int expected_moves = 2;

			if (parts[1] != "startpos") {
				if (parts.size() < 7) {
					neocortex_warn("Invalid UCI: not enough FEN fields for position\n");
					continue;
				}

				std::string fen = util::join(std::vector<std::string>(parts.begin() + 1, parts.begin() + 7), " ");

				pos = fen;
				expected_moves = 6;
			}

			if (expected_moves < (int) parts.size()) {
				if (parts[expected_moves] != "moves") {
					neocortex_warn("Invalid UCI: expected 'moves', read '%s'\n", parts[expected_moves].c_str());
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
						neocortex_warn("Invalid UCI: unmatched move '%s'\n", parts[i].c_str());
						break;
					}

					pos.make_move(matched_move);
				}
			}

			searcher.load(pos);
			neocortex_debug("Loaded position %s\n", pos.to_fen().c_str());
		}
	}
}
