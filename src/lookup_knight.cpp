#include "lookup_knight.h"
#include "square.h"

using namespace nc2;

static std::vector<Move> _nc2_lookup_knight_table[64];
static u64 _nc2_lookup_knight_attack_table[64];

void lookup::initialize_knight_lookup() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            for (int a = -1; a <= 1; a += 2) {
                for (int b = -2; b <= 2; b += 4) {
                    if (r + a >= 0 && r + a < 8 && f + b >= 0 && f + b < 8) {
                        _nc2_lookup_knight_table[square::at(r, f)].push_back(Move(square::at(r, f), square::at(r + a, f + b)));
                        _nc2_lookup_knight_attack_table[square::at(r, f)] |= square::mask(square::at(r + a, f + b));
                    }

                    if (r + b >= 0 && r + b < 8 && f + a >= 0 && f + a < 8) {
                        _nc2_lookup_knight_table[square::at(r, f)].push_back(Move(square::at(r, f), square::at(r + b, f + a)));
                        _nc2_lookup_knight_attack_table[square::at(r, f)] |= square::mask(square::at(r + b, f + a));
                    }
                }
            }
        }
    }
}

const std::vector<Move>& lookup::knight_moves(u8 s) {
    return _nc2_lookup_knight_table[s];
}

u64 lookup::knight_attacks(u8 s) {
    return _nc2_lookup_knight_attack_table[s];
}
