#include "square.h"

using namespace nc;

Square::Square() {
    index = rank = file = -1;
}

Square::Square(int rank, int file) {
    this->rank = rank;
    this->file = file;

    update();
}

Square::Square(std::string uci) {
    if (uci.size() != 2) {
        index = rank = file = -1;
        return;
    }

    file = uci[0] + 1 - 'a';
    rank = uci[1] + 1 - '1';

    update();
}

void Square::shift(int ranks, int files) {
    rank += ranks;
    file += files;

    update();
}

int Square::get_rank() {
    return rank;
}

int Square::get_file() {
    return file;
}

int Square::get_index() {
    return index;
}

bool Square::is_valid() {
    return (index > 0);
}

Square::operator int() {
    return get_index();
}

Square::operator bool() {
    return is_valid();
}

void Square::update() {
    if (rank < 1 || rank > 8 || file < 1 || file > 8) {
        index = rank = file = -1;
    }

    index = (rank - 1) * 8 + file - 1;
}
