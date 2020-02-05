#include "lookup_king.h"
#include "square.h"

using namespace nc2;

static std::vector<Move> _nc2_lookup_king_table[64];
static u64 _nc2_lookup_king_attack_table[64];

void lookup::initialize_king_lookup() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (!dx && !dy) continue;

                    if (f + dx < 0 || f + dx >= 8) continue;
                    if (r + dy < 0 || r + dy >= 8) continue;

                    _nc2_lookup_king_table[square::at(r, f)].push_back(Move(square::at(r, f), square::at(r + dy, f + dx)));
                    _nc2_lookup_king_attack_table[square::at(r, f)] |= square::mask(square::at(r + dy, f + dx));
                }
            }
        }
    }
}

const std::vector<Move>& lookup::king_moves(u8 s) {
    return _nc2_lookup_king_table[s];
}

u64 lookup::king_attacks(u8 s) {
    return _nc2_lookup_king_attack_table[s];
}
