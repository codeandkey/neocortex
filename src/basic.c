#include "basic.h"
#include "util.h"

static nc_bb _nc_knight_lookup[64];
static nc_bb _nc_king_lookup[64];

void nc_basic_init() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            nc_square src = nc_square_at(r, f);

            /* Built knight lookup symmetrically. */
            for (int a = -2; a <= 2; a += 4) {
                for (int b = -1; b <= 1; b += 2) {
                    int dra = r + a, dfa = f + b;
                    int drb = r + b, dfb = f + a;

                    if (dra >= 0 && dra < 8 && dfa >= 0 && dfa < 8) {
                        _nc_knight_lookup[src] |= nc_bb_mask(nc_square_at(dra, dfa));
                    }

                    if (drb >= 0 && drb < 8 && dfb >= 0 && dfb < 8) {
                        _nc_knight_lookup[src] |= nc_bb_mask(nc_square_at(drb, dfb));
                    }
                }
            }

            /* Build the king lookup with two quick loops. */
            for (int dr = r - 1; dr <= r + 1; ++dr) {
                for (int df = f - 1; df <= f + 1; ++df) {
                    if (dr == r && df == f) continue;
                    if (dr < 0 || dr >= 8) continue;
                    if (df < 0 || df >= 8) continue;
                    _nc_king_lookup[src] |= nc_bb_mask(nc_square_at(dr, df));
                }
            }
        }
    }

    nc_debug("Built king/knight bitboard lookups.");
}

nc_bb nc_basic_knight_attacks(nc_square src) {
    return _nc_knight_lookup[src];
}

nc_bb nc_basic_king_attacks(nc_square src) {
    return _nc_king_lookup[src];
}
