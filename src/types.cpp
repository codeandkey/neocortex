#include "types.h"

std::string nc2::bitboard_to_string(u64 bb) {
    std::string out;

    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            if ((bb >> (r * 8 + f)) & 1) {
                out += '#';
            } else {
                out += '.';
            }
        }

        out += '\n';
    }


    return out;
}
