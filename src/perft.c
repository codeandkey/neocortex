#include "perft.h"
#include "position.h"
#include "timer.h"

static int _nc_perft_nodes;
static int _nc_perft_leaves;
static int _nc_perft_captures;

static void _nc_perft_pos(nc_position* p, int depth);

void nc_perft_run(FILE* out, int depth) {
    for (int i = 0; i <= depth; ++i) {
        fprintf(out, "perft[%d]: starting..\n", i);
        nc_timepoint start = nc_timer_current();
        nc_position p;
        nc_position_init(&p);

        _nc_perft_nodes = 0;
        _nc_perft_leaves = 0;
        _nc_perft_captures = 0;

        _nc_perft_pos(&p, i);

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

    nc_movelist next_moves;
    nc_movelist_clear(&next_moves);
    nc_position_legal_moves(p, &next_moves);

    for (int i = 0; i < next_moves.len; ++i) {
        nc_move move = next_moves.moves[i];

        if (depth == 1 && (move & NC_CAPTURE)) ++_nc_perft_captures;
        nc_position_make_move(p, move);
        _nc_perft_pos(p, depth - 1);
        nc_position_unmake_move(p, move);
    }
}
