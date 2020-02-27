#include "pst.h"
#include "util.h"

static nc_eval _nc_pst_material_values[6] = {
    100, 500, 300, 300, 740, 0,
};

static nc_eval _nc_pst[2][6][64] = {
    {
        { /* push pawns to the end, encourage good structure and center control */
             0,  0,  0,  0,  0,  0,  0,  0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 20, 30, 40, 40, 30, 20, 10,
             5, 10, 15, 25, 25, 15, 10,  5,
             0,  0,  0, 20, 20,  0,  0,  0,
             4, -5,-10,  0,  0,-10, -5,  0,
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

void nc_pst_init() {
    for (int t = 0; t < 6; ++t) {
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 8; ++f) {
                /* incorporate material values into pst */
                _nc_pst[NC_WHITE][t][nc_square_at(r, f)] += _nc_pst_material_values[t];

                int br = 7 - r;
                _nc_pst[NC_BLACK][t][nc_square_at(br, f)] = _nc_pst[NC_WHITE][t][nc_square_at(r, f)];
            }
        }
    }

    nc_debug("Initialized PST data.");
}

void nc_pst_remove(nc_pst_eval* pst, nc_piece p, nc_square sq) {
    nc_color c = nc_piece_color(p);
    nc_ptype pt = nc_piece_type(p);

    pst->mg[c] -= _nc_pst[c][pt][sq];
}

void nc_pst_add(nc_pst_eval* pst, nc_piece p, nc_square sq) {
    nc_color c = nc_piece_color(p);
    nc_ptype pt = nc_piece_type(p);

    pst->mg[c] += _nc_pst[c][pt][sq];
}
