#include "occtable.h"

#include <cstring>

using namespace nc;

Occtable::Occtable() {
    memset(this, 0, sizeof *this);
}

Occtable Occtable::standard() {
    Occtable out;

    for (int f = 0; f < 8; ++f) {
        out.flip(Square(0, f));
        out.flip(Square(1, f));
        out.flip(Square(6, f));
        out.flip(Square(7, f));
    }

    return out;
}

void Occtable::flip(Square s) {
    u8 rb = (1 << s.get_rank());

    ranks[s.get_rank()] ^= (1 << s.get_file());
    files[s.get_file()] ^= rb;
    diags[s.get_diag()] ^= rb;
    antidiags[s.get_antidiag()] ^= rb;
}

u8 Occtable::get_rank(int r) {
    return (ranks[r] >> 1) & 0x3F;
}

u8 Occtable::get_file(int f) {
    return (files[f] >> 1) & 0x3F;
}

u8 Occtable::get_diag(int d) {
    return (diags[d] >> 1) & 0x3F;
}

u8 Occtable::get_antidiag(int d) {
    return (antidiags[d] >> 1) & 0x3F;
}
