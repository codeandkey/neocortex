#include "piece.h"

#include <cctype>

using namespace nc;

Piece::Piece() {
    uci = col = type = 0;
}

Piece::Piece(char uci) {
    char t = tolower(uci);
    set((uci == t) ? 'b' : 'w', t);
}

Piece::Piece(char col, char type) {
    set(col, type);
}

void Piece::set(char col, char type) {
    this->col = col;
    this->type = type;

    if (this->col == 'w') {
        this->uci = toupper(this->type);
    } else if (this->col == 'b') {
        this->uci = this->type;
    } else {
        this->uci = this->col = this->type = 0;
    }

    switch (this->type) {
    case 'r':
    case 'b':
    case 'p':
    case 'k':
    case 'q':
    case 'n':
        break;
    default:
        this->uci = this->col = this->type = 0;
    }
}

char Piece::get_uci() {
    return uci;
}

char Piece::get_type() {
    return type;
}

char Piece::get_color() {
    return col;
}

bool Piece::is_valid() {
    return (uci != 0);
}

Piece::operator char() {
    return get_uci();
}

Piece::operator bool() {
    return is_valid();
}
