#include "pst.h"
#include "util.h"
#include "eval_consts.h"

#include <string.h>

static nc_eval _nc_pst_mg_value[6] = {
    NC_EVAL_PAWN_MG,
    NC_EVAL_ROOK_MG,
    NC_EVAL_KNIGHT_MG,
    NC_EVAL_BISHOP_MG,
    NC_EVAL_QUEEN_MG,
    0,
};

static nc_eval _nc_pst_eg_value[6] = {
    NC_EVAL_PAWN_EG,
    NC_EVAL_ROOK_EG,
    NC_EVAL_KNIGHT_EG,
    NC_EVAL_BISHOP_EG,
    NC_EVAL_QUEEN_EG,
    0,
};

static const float _nc_pst_total_mg_mat = 16 * NC_EVAL_PAWN_MG \
                                          + 4 * NC_EVAL_KNIGHT_MG \
                                          + 4 * NC_EVAL_BISHOP_MG \
                                          + 4 * NC_EVAL_ROOK_MG \
                                          + 2 * NC_EVAL_QUEEN_MG;

static nc_eval _nc_pst_mg[2][6][64] = {
    {
        { /* push pawns to the end, encourage good structure and center control */
             0,  0,  0,  0,  0,  0,  0,  0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 20, 30, 40, 40, 30, 20, 10,
             5, 10, 15, 25, 25, 15, 10,  5,
             0,  0,  0, 20, 20,  0,  0,  0,
             4, -5,-10,  0,  0,-10, -5,  4,
             4, 10, 10,-25,-25, 10, 10,  4,
             0,  0,  0,  0,  0,  0,  0,  0,
        },
        { /* keep rooks mobile, occupy the 7th rank */
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, 10, 10, 10, 10, 5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            0, 0, 0, 5, 5, 0, 0, 0,
        },
        { /* keep knights controlling the center and away from corners */
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20,   0,   0,   0,   0, -20, -40,
            -30,   0,  10,  15,  15,  10,   0, -30,
            -30,   5,  15,  20,  20,  15,   5, -30,
            -30,   0,  15,  20,  20,  15,   0, -30,
            -30,   5,  10,  15,  15,  10,   5, -30,
            -40, -20,   0,   5,   5,   0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50,
        },
        { /* prefer bishops on nice squares */
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -10,   0,   5,  10,  10,   5,   0, -10,
            -10,   5,   5,  10,  10,   5,   5, -10,
            -10,   0,   10,  10,  10,   10,   0, -10,
            -10,   10,   10,  10,  10,   10,   10, -10,
            -10,   5,   0,  0,  0,  0,   5, -10,
            -20, -10, -10, -10, -10, -10, -10, -20,
        },
        { /* keep queen away from edges */
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -5, 0, 5, 5, 5, 5, 0, -5,
            0, 0, 5, 5, 5, 5, 0, -5,
            -10, 5, 5, 5, 5, 5, 0, -10,
            10,0, 5, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20,
        },
        { /* keep king in safe squares */
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -20, -30, -30, -40, -40, -30, -20, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            20, 20, 0, 0, 0, 0, 20, 20,
            20, 30, 10, 0, 0, 10, 30, 20,
        },
    }, {{0}},
};

static nc_eval _nc_pst_eg[2][6][64] = {
    {
        { /* bring pawns to the end */
             0,  0,  0,  0,  0,  0,  0,  0,
            100, 100, 100, 100, 100, 100, 100, 100,
            80, 80, 80, 80, 80, 80, 80, 80,
            60, 60, 60, 60, 60, 60, 60, 60,
            40, 40, 40, 40, 40, 40, 40, 40,
            20, 20, 20, 20, 20, 20, 20, 20,
            0, 0, 0, 0, 0, 0, 0, 0,
             0,  0,  0,  0,  0,  0,  0,  0,
        },
        { /* keep rooks mobile, occupy the 7th rank */
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, 10, 10, 10, 10, 5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            0, 0, 0, 5, 5, 0, 0, 0,
        },
        { /* keep knights controlling the center and away from corners */
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20,   0,   0,   0,   0, -20, -40,
            -30,   0,  10,  15,  15,  10,   0, -30,
            -30,   5,  15,  20,  20,  15,   5, -30,
            -30,   0,  15,  20,  20,  15,   0, -30,
            -30,   5,  10,  15,  15,  10,   5, -30,
            -40, -20,   0,   5,   5,   0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50,
        },
        { /* prefer bishops on nice squares */
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -10,   0,   5,  10,  10,   5,   0, -10,
            -10,   5,   5,  10,  10,   5,   5, -10,
            -10,   0,   10,  10,  10,   10,   0, -10,
            -10,   10,   10,  10,  10,   10,   10, -10,
            -10,   5,   0,  0,  0,  0,   5, -10,
            -20, -10, -10, -10, -10, -10, -10, -20,
        },
        { /* keep queen away from edges */
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -5, 0, 5, 5, 5, 5, 0, -5,
            0, 0, 5, 5, 5, 5, 0, -5,
            -10, 5, 5, 5, 5, 5, 0, -10,
            10,0, 5, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20,
        },
        { /* bring king to the center */
            0, 0, 10, 20, 20, 10, 0, 0,
            0, 10, 20, 30, 30, 20, 10, 0,
            10, 20, 30, 40, 40, 30, 20, 10,
            20, 30, 40, 50, 50, 40, 30, 20,
            20, 30, 40, 50, 50, 40, 30, 20,
            10, 20, 30, 40, 40, 30, 20, 10,
            0, 10, 20, 30, 30, 20, 10, 0,
            0, 0, 10, 20, 20, 10, 0, 0,
        },
    }, {{0}},
};

void nc_pst_init() {
    /* The psts are defined in reverse rank order. */
    /* Copy over to black first and then reverse back. */

    for (int t = 0; t < 6; ++t) {
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 8; ++f) {
                _nc_pst_mg[NC_BLACK][t][nc_square_at(r, f)] = _nc_pst_mg[NC_WHITE][t][nc_square_at(r, f)];
                _nc_pst_eg[NC_BLACK][t][nc_square_at(r, f)] = _nc_pst_eg[NC_WHITE][t][nc_square_at(r, f)];

                int br = 7 - r;
                _nc_pst_mg[NC_WHITE][t][nc_square_at(br, f)] = _nc_pst_mg[NC_BLACK][t][nc_square_at(r, f)];
                _nc_pst_eg[NC_WHITE][t][nc_square_at(br, f)] = _nc_pst_eg[NC_BLACK][t][nc_square_at(r, f)];
            }
        }
    }

    nc_debug("Initialized PST data.");
}

void nc_pst_zero(nc_pst_eval* pst) {
    memset(pst, 0, sizeof *pst);
}

void nc_pst_remove(nc_pst_eval* pst, nc_piece p, nc_square sq) {
    nc_color c = nc_piece_color(p);
    nc_ptype pt = nc_piece_type(p);

    pst->mg[c] -= _nc_pst_mg[c][pt][sq];
    pst->mg_material[c] -= _nc_pst_mg_value[pt];

    pst->eg[c] -= _nc_pst_eg[c][pt][sq];
    pst->eg_material[c] -= _nc_pst_eg_value[pt];
}

void nc_pst_add(nc_pst_eval* pst, nc_piece p, nc_square sq) {
    nc_color c = nc_piece_color(p);
    nc_ptype pt = nc_piece_type(p);

    pst->mg[c] += _nc_pst_mg[c][pt][sq];
    pst->mg_material[c] += _nc_pst_mg_value[pt];

    pst->eg[c] += _nc_pst_eg[c][pt][sq];
    pst->eg_material[c] += _nc_pst_eg_value[pt];
}

nc_eval nc_pst_get_score(nc_pst_eval* pst, nc_color color_to_move, float phase) {
    int other = nc_colorflip(color_to_move);

    nc_eval mg = (nc_eval) (phase * (pst->mg[color_to_move] + pst->mg_material[color_to_move]));
    nc_eval eg = (nc_eval) ((1.0f - phase) * (pst->eg[color_to_move] + pst->eg_material[color_to_move]));

    nc_eval omg = (nc_eval) (phase * (pst->mg[other] + pst->mg_material[other]));
    nc_eval oeg = (nc_eval) ((1.0f - phase) * (pst->eg[other] + pst->eg_material[other]));

    return mg + eg - (omg + oeg);
}

float nc_pst_get_phase(nc_pst_eval* pst) {
    nc_eval mg_total = pst->mg_material[NC_WHITE] + pst->mg_material[NC_BLACK];

    return (float) (mg_total) / _nc_pst_total_mg_mat;
}
