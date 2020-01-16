#include "square.h"

using namespace nc;

Square::Square() {
    unset();
}

Square::Square(std::string uci) {
    if (uci.size() != 2) {
        unset();
        return;
    }

    file = uci[0] - 'a';
    rank = uci[1] - '1';

    update();
}

Square::Square(int rank, int file) {
    this->rank = rank;
    this->file = file;

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

int Square::get_diag() {
    return rank - file + 7;
}

int Square::get_antidiag() {
    return rank + file;
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
    if (rank < 0 || rank > 7 || file < 0 || file > 7) {
        unset();
        return;
    }

    index = rank * 8 + file;
}

void Square::unset() {
    index = rank = file = -1;
}

std::string Square::to_string() {
    if (index < 0) return "-";

    std::string out;
    out += (file + 'a');
    out += (rank + '1');

    return out;
}

Square::operator std::string() {
    return to_string();
}
