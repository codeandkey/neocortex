/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "uci.h"
#include "log.h"
#include "util.h"
#include "search.h"
#include "position.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace neocortex;

static void write_info(search::SearchInfo inf) {
	std::cout << inf.to_string() << "\n";
}

static void write_bestmove(Move m) {
	std::cout << "bestmove " << m.to_uci() << "\n";
}

static std::string read_command() {
	std::string output;
	std::getline(std::cin, output);

	return util::trim(output);
}

static int safe_parseint(std::vector<std::string> parts, size_t ind) {
	if (ind >= parts.size()) {
		throw std::runtime_error("Expected argument!");
	}

	neocortex_debug("Parsing %s\n", parts[ind].c_str());

	return std::stoi(parts[ind]);
}

void uci::start() {
	std::string handshake  = read_command();

	if (handshake != "uci") {
		throw std::runtime_error(util::format("Invalid UCI: expected 'uci', read '%s'", handshake.c_str()));
	}

	std::cout << "uciok\n";
	std::cout << "id name " << uci::NAME << "\n";
	std::cout << "id author " << uci::AUTHOR << "\n";

	search::Search searcher;

	while (true) {
		std::string command = read_command();
		std::vector<std::string> parts = util::split(command, ' ');

		if (!parts.size()) continue;
			
		if (parts[0] == "quit") {
			break;
		}
		else if (parts[0] == "isready") {
			std::cout << "readyok\n";
		}
		else if (parts[0] == "stop") {
			searcher.stop();
		}
		else if (parts[0] == "go") {
			int wtime = -1, btime = -1, winc = -1, binc = -1, depth = -1, movetime = -1, movestogo = -1;
			bool infinite = false;

			/* Parse UCI options. */
			for (size_t i = 1; i < parts.size(); ++i) {
				if (isdigit(parts[i][0])) continue;

				if (parts[i] == "wtime") wtime = safe_parseint(parts, i + 1);
				else if (parts[i] == "btime") btime = safe_parseint(parts, i + 1);
				else if (parts[i] == "winc") winc = safe_parseint(parts, i + 1);
				else if (parts[i] == "binc") binc = safe_parseint(parts, i + 1);
				else if (parts[i] == "depth") depth = safe_parseint(parts, i + 1);
				else if (parts[i] == "movetime") movetime = safe_parseint(parts, i + 1);
				else if (parts[i] == "infinite") infinite = true;
				else if (parts[i] == "movestogo") {} /* ignore movestogo */
				else {
					throw util::fmterr("Invalid argument: %s", parts[i].c_str());
				}
			}

			searcher.go(write_info, write_bestmove, wtime, btime, winc, binc, depth, movetime, infinite);
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
					neocortex_warn("Invalid UCI: not enough FEN fields for position [need %d, have %d]\n", 6, parts.size());
					continue;
				}

				std::string fen = util::join(std::vector<std::string>(parts.begin() + 1, parts.begin() + 7), " ");

				pos = fen;
				expected_moves = 7;
			}

			if (expected_moves < (int) parts.size()) {
				if (parts[expected_moves] != "moves") {
					neocortex_warn("Invalid UCI: expected 'moves', read '%s'\n", parts[expected_moves].c_str());
					continue;
				}

				for (size_t i = expected_moves + 1; i < parts.size(); ++i) {
					Move pl_moves[MAX_PL_MOVES], matched_move = Move::null;
					int num_pl_moves;

					num_pl_moves = pos.pseudolegal_moves(pl_moves);

					for (int j = 0; j < num_pl_moves; ++j) {
						if (pl_moves[j].match_uci(parts[i])) {
							assert(matched_move == Move::null);
							matched_move = pl_moves[j];
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
