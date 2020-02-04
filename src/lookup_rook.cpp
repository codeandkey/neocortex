#include "lookup_rook.h"
#include "square.h"

using namespace nc2;

std::vector<Move> _nc2_lookup_rook_table[64][64][64]; /* [square][rank_rocc][file_rocc] */

void lookup::initialize_rook_lookup() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            u8 s = square::at(r, f);

            /* The loops generating the moves are mutually exclusive so this isn't very efficient --
             * however the actual performance cost is negligible. */
            for (u8 rank_rocc = 0; rank_rocc < 64; ++rank_rocc) {
                for (u8 file_rocc = 0; file_rocc < 64; ++file_rocc) {
                    std::vector<Move>* dst = &_nc2_lookup_rook_table[s][rank_rocc][file_rocc];

                    u8 file_occ = file_rocc << 1;
                    u8 rank_occ = rank_rocc << 1;

                    /* Walk north */
                    for (int cr = r + 1; cr < 8; ++cr) {
                        dst->push_back(Move(s, square::at(cr, f)));
                        if ((rank_occ >> cr) & 1) break;
                    }

                    /* Walk south */
                    for (int cr = r - 1; cr >= 0; --cr) {
                        dst->push_back(Move(s, square::at(cr, f)));
                        if ((rank_occ >> cr) & 1) break;
                    }

                    /* Walk east */
                    for (int cf = f + 1; cf < 8; ++cf) {
                        dst->push_back(Move(s, square::at(r, cf)));
                        if ((file_occ >> cf) & 1) break;
                    }

                    /* Walk west */
                    for (int cf = f - 1; cf >= 0; --cf) {
                        dst->push_back(Move(s, square::at(r, cf)));
                        if ((file_occ >> cf) & 1) break;
                    }
                }
            }
        }
    }
}

const std::vector<Move>& lookup::rook_moves(u8 s, Occboard* occ) {
    u8 r = square::rank(s), f = square::file(s);
    u8 rank_rocc = Occboard::to_rocc(occ->get_rank(r));
    u8 file_rocc = Occboard::to_rocc(occ->get_file(f));

    return _nc2_lookup_rook_table[s][rank_rocc][file_rocc];
}

