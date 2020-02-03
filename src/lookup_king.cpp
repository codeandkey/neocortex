#include "lookup_king.h"
#include "square.h"

using namespace nc2;

std::vector<Move> _nc2_lookup_king_table[64];

void lookup::initialize_king_lookup() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (!dx && !dy) continue;

                    if (f + dx < 0 || f + dx >= 8) continue;
                    if (r + dy < 0 || r + dy >= 8) continue;

                    _nc2_lookup_king_table[square::at(r, f)].push_back(Move(square::at(r, f), square::at(r + dy, f + dx)));
                }
            }
        }
    }
}

std::vector<Move> lookup::king_moves(u8 s) {
    return _nc2_lookup_king_table[s];
}
