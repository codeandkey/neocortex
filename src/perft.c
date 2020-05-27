#include "perft.h"
#include "position.h"
#include "timer.h"
#include "movegen.h"

static int _nc_perft_nodes;
static int _nc_perft_leaves;
static int _nc_perft_captures;

static void _nc_perft_pos(nc_position* p, int depth);

void nc_perft_run(FILE* out, nc_position* p, int depth) {
	for (int i = 0; i <= depth; ++i) {
		fprintf(out, "perft[%d]: starting..\n", i);
		nc_timepoint start = nc_timer_current();

		_nc_perft_nodes = 0;
		_nc_perft_leaves = 0;
		_nc_perft_captures = 0;

		_nc_perft_pos(p, i);

		float final = nc_timer_elapsed_s(start);

		fprintf(out, "perft[%d]: results for depth %d: %d total, %d leaves, %d captures\n", i, i, _nc_perft_nodes, _nc_perft_leaves, _nc_perft_captures);
		fprintf(out, "perft[%d]: finished in %.2f seconds, rate %d nodes per second\n", i, final, (int) ((float) _nc_perft_nodes / final));
	}
}

void _nc_perft_pos(nc_position* p, int depth) {
	++_nc_perft_nodes;

	if (!depth) {
		++_nc_perft_leaves;
		return;
	}

	nc_movegen gen;
	nc_move nextmove;

	nc_movegen_start_gen(p, &gen);
	
	while (nc_movegen_next_move(p, &gen, &nextmove)) {
		if (nc_position_make_move(p, nextmove)) {
			_nc_perft_pos(p, depth - 1);
			if (depth == 1 && (nextmove & NC_CAPTURE)) ++_nc_perft_captures;
		}

		nc_position_unmake_move(p, nextmove);
	}
}
