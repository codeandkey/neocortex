#include "move.h"

#include <stdexcept>

using namespace nc;

Move::Move(std::string uci) {
    switch (uci.size()) {
    case 5:
        promote_type = uci[4];
    case 4:
        from = uci.substr(0, 2);
        to = uci.substr(2, 2);
        break;
    default:
        throw std::runtime_error("Invalid move UCI string!");
    }
}

Move::Move(Square from, Square to, char promote_type) {
    this->from = from;
    this->to = to;
    this->promote_type = promote_type;
}

std::string Move::to_string() {
    std::string out;

    out += from.to_string();
    out += to.to_string();

    if (promote_type) {
        out += promote_type;
    }

    return out;
}

Move::operator std::string() {
    return to_string();
}

Square Move::get_from() {
    return from;
}

Square Move::get_to() {
    return to;
}

char Move::get_promote_type() {
    return promote_type;
}
