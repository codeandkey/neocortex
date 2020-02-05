#include "piece.h"

#include <cassert>

using namespace nc2;

static char _nc2_uci_chars[] = "PpNnBbRrKkQq----";
static char _nc2_type_chars[] = "pnbrkq";

u8 piece::make(u8 type, u8 color) {
    return (type << 1) | color;
}

u8 piece::type(u8 p) {
    return p >> 1;
}

u8 piece::color(u8 p) {
    return p & 1;
}

char piece::uci(u8 p) {
    assert(p <= 15);
    return _nc2_uci_chars[p];
}

char piece::type_char(u8 t) {
    assert(t < sizeof _nc2_type_chars);
    return _nc2_type_chars[t];
}

bool piece::exists(u8 p) {
    return (p != piece::null);
}
