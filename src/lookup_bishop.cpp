#include "lookup_bishop.h"
#include "square.h"

using namespace nc2;

std::vector<Move> _nc2_lookup_bishop_table[64][64][64]; /* [square][diag_rocc][antidiag_rocc] */

void lookup::initialize_bishop_lookup() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            u8 s = square::at(r, f);

            /* The loops generating the moves are mutually exclusive so this isn't very efficient --
             * however the actual performance cost is negligible. */
            for (u8 diag_rocc = 0; diag_rocc < 64; ++diag_rocc) {
                for (u8 antidiag_rocc = 0; antidiag_rocc < 64; ++antidiag_rocc) {
                    std::vector<Move>* dst = &_nc2_lookup_bishop_table[s][diag_rocc][antidiag_rocc];

                    u8 diag_occ = diag_rocc << 1;
                    u8 antidiag_occ = antidiag_rocc << 1;

                    /* Walk northeast (diag) */
                    for (int d = 1; r + d < 8 && f + d < 8; ++d) {
                        dst->push_back(Move(s, square::at(r + d, f + d)));
                        if ((diag_occ >> (r + d)) & 1) break;
                    }

                    /* Walk southwest (diag) */
                    for (int d = 1; r - d >= 0 && f - d >= 0; ++d) {
                        dst->push_back(Move(s, square::at(r - d, f - d)));
                        if ((diag_occ >> (r - d)) & 1) break;
                    }

                    /* Walk northwest (antidiag) */
                    for (int d = 1; r + d < 8 && f - d >= 0; ++d) {
                        dst->push_back(Move(s, square::at(r + d, f - d)));
                        if ((antidiag_occ >> (r + d)) & 1) break;
                    }

                    /* Walk southeast (diag) */
                    for (int d = 1; r - d >= 0 && f + d < 8; ++d) {
                        dst->push_back(Move(s, square::at(r - d, f + d)));
                        if ((antidiag_occ >> (r - d)) & 1) break;
                    }
                }
            }
        }
    }
}

const std::vector<Move>& lookup::bishop_moves(u8 s, Occboard* occ) {
    u8 d = square::diag(s), ad = square::antidiag(s);
    u8 diag_rocc = Occboard::to_rocc(occ->get_diag(d));
    u8 antidiag_rocc = Occboard::to_rocc(occ->get_antidiag(ad));

    return _nc2_lookup_bishop_table[s][diag_rocc][antidiag_rocc];
}

