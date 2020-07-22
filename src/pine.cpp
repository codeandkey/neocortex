#include "attacks.h"
#include "log.h"
#include "platform.h"
#include "tt.h"
#include "zobrist.h"

#include <iostream>

using namespace pine;

int main(int argc, char* argv) {
#ifdef PINE_DEBUG
	log::set_level(log::DEBUG);
#endif

	try {
		pine_debug("Starting pine %s\n", PINE_VERSION);
		pine_debug("Build: %s\n", PINE_BUILDTIME);
		pine_debug("Platform: %s\n", PINE_PLATFORM);

#ifdef PINE_DEBUG
		pine_warn("Compile time debug enabled. Performance will be slower!\n");
#endif

		zobrist::init();
		attacks::init();
		tt::init();

		Position p("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");

		pine_debug("global_occ:\n%s", bb::to_string(p.get_board().get_global_occ()).c_str());
		pine_debug("pretty:\n%s", p.get_board().to_pretty().c_str());

		movegen::Generator g(p);
		int num_moves = 0;

		while (true) {
			std::list<Move> movelist = g.generate();
			if (!movelist.size()) break;

			for (auto next_move : movelist) {
				std::string move_str = next_move.to_pgn(p.get_board());

				bool res = p.make_move(next_move);
				if (res) pine_debug("move: %s %s\n", move_str.c_str(), res ? "legal" : "illegal");
				p.unmake_move(next_move);
				++num_moves;
			}
		}

		pine_debug("generated %d moves\n", num_moves);

		perft::write_run(p, 3, std::cout);
	}
	catch (std::exception& e) {
		pine_error("Unhandled exception: %s", e.what());
	}

	(void)getchar();

	return 0;
}