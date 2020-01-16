#include "bitboard.h"

using namespace nc;

std::string bitboard::to_string(u64 b) {
    std::string out;

    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            if ((b >> (f+8*r))&1) {
                out += 'x';
            } else {
                out += '.';
            }
        }

        out += '\n';
    }

    return out;
}
