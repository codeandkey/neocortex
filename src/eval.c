#include "eval.h"
#include "eval_consts.h"

static nc_eval _nc_material_mg[] = {
    NC_EVAL_PAWN_MG,
    NC_EVAL_ROOK_MG,
    NC_EVAL_KNIGHT_MG,
    NC_EVAL_BISHOP_MG,
    NC_EVAL_QUEEN_MG,
};

static nc_eval _nc_material_eg[] = {
    NC_EVAL_PAWN_EG,
    NC_EVAL_ROOK_EG,
    NC_EVAL_KNIGHT_EG,
    NC_EVAL_BISHOP_EG,
    NC_EVAL_QUEEN_EG,
};

static const nc_eval nc_eval_total_npm_mg = 4 * (NC_EVAL_ROOK_MG + NC_EVAL_KNIGHT_MG + NC_EVAL_BISHOP_MG) + 2 * NC_EVAL_QUEEN_MG;

//static void nc_eval_position_king_safety(nc_position* p, nc_position_eval* dst);
static void nc_eval_position_material(nc_position* p, nc_position_eval* dst);

void nc_eval_position(nc_position* p, nc_position_eval* dst) {
//    nc_eval_position_king_safety(p, dst);
    nc_eval_position_material(p, dst);

    /* compute phase */
    dst->phase = (dst->material_nonpawn_mg[NC_WHITE] + dst->material_nonpawn_mg[NC_BLACK]) * 255 / nc_eval_total_npm_mg;

    dst->score += dst->phase * (dst->material_pawn_mg[p->color_to_move] + dst->material_nonpawn_mg[p->color_to_move]);
    dst->score += (255 - dst->phase) * (dst->material_pawn_eg[p->color_to_move] + dst->material_nonpawn_eg[p->color_to_move]);

    dst->score -= dst->phase * (dst->material_pawn_mg[!p->color_to_move] + dst->material_nonpawn_mg[!p->color_to_move]);
    dst->score -= (255 - dst->phase) * (dst->material_pawn_eg[!p->color_to_move] + dst->material_nonpawn_eg[!p->color_to_move]);
}

void nc_eval_position_material(nc_position* p, nc_position_eval* dst) {
    for (int c = 0; c < 2; ++c) {
        int pawn_count = __builtin_popcountll(p->color[c] & p->piece[NC_PAWN]);

        dst->material_pawn_mg[c] = pawn_count * NC_EVAL_PAWN_MG;
        dst->material_pawn_eg[c] = pawn_count * NC_EVAL_PAWN_EG;

        dst->material_nonpawn_mg[c] = dst->material_nonpawn_eg[c] = 0;

        /* rook, knight, bishop, queen */
        for (int t = 1; t < 5; ++t) {
            int count = __builtin_popcountll(p->color[c] & p->piece[t]);

            dst->material_nonpawn_mg[c] += count * _nc_material_mg[t];
            dst->material_nonpawn_eg[c] += count * _nc_material_eg[t];
        }
    }
}
