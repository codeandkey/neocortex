#include "move.h"
#include "square.h"
#include "piece.h"

using namespace nc2;

Move::Move() : from(square::null), to(square::null), ptype(piece::Type::NONE) {}

Move::Move(u8 from, u8 to, u8 ptype) : from(from), to(to), ptype(ptype) {}

u8 Move::get_from() {
    return from;
}

u8 Move::get_to() {
    return to;
}

u8 Move::get_ptype() {
    return ptype;
}

std::string Move::to_string() {
    std::string out = square::to_string(from);
    out += square::to_string(to);

    switch (ptype) {
    case piece::Type::PAWN:
        out += 'p';
        break;
    case piece::Type::ROOK:
        out += 'r';
        break;
    case piece::Type::BISHOP:
        out += 'b';
        break;
    case piece::Type::KNIGHT:
        out += 'n';
        break;
    case piece::Type::QUEEN:
        out += 'q';
        break;
    case piece::Type::KING:
        out += 'k';
        break;
    }

    return out;
}
