/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "perft.h"
#include "util.h"
#include "log.h"

#include <cassert>
#include <iostream>

using namespace neocortex;

static perft::results current_results;
static void perft_movegen(Position& p, int depth);

std::string perft::results::header() {
	return "| depth |     nodes |   captures |   checks | castles |   time |       nps |\n";
}

std::string perft::results::to_row(int depth) {
	return util::format("| %5d | %9d | %10d | %8d | %7d | %6.2f | %9d |\n", depth, nodes, captures, checks, castles, totaltime, nps);
}

perft::results perft::run(Position& p, int depth) {
	if (depth < 0) {
		throw util::fmterr("Invalid perft depth %d", depth);
	}

	current_results = perft::results();

	util::time_point now = util::time_now();

	perft_movegen(p, depth);

	current_results.totaltime = util::time_elapsed(now);
	current_results.nps = (unsigned long) (current_results.nodes / current_results.totaltime);

	return current_results;
}

void perft::start(Position& p, int depth, std::ostream& out) {
	std::cout << "| depth |     nodes |   captures |   checks | castles |   time |       nps |\n";

	for (int i = 1; i <= depth; ++i) {
		std::cout << perft::run(p, i).to_row(i);
	}
}

void perft_movegen(Position& p, int depth) {
	if (!depth) {
		current_results.nodes++;

		if (p.capture()) current_results.captures++;
		if (p.check()) current_results.checks++;
		if (p.castle()) current_results.castles++;
		if (p.en_passant()) current_results.en_passant++;
		if (p.promotion()) current_results.promotions++;
	
		return;
	}

	Move pl_moves[MAX_PL_MOVES];
	int num_pl_moves = p.pseudolegal_moves(pl_moves);

	for (int i = 0; i < num_pl_moves; ++i) {
		if (p.make_move(pl_moves[i])) {
			perft_movegen(p, depth - 1);
		}

		p.unmake_move(pl_moves[i]);
	}
}
