#include "game.h"

#include <stdexcept>

using namespace nc;

Game::Game() {
    update_next_moves();
}

Game::Game(std::string fen) : current_position(fen) {
    update_next_moves();
}

Position::Transition Game::apply(Move move) {
    for (auto i : legal_next_moves) {
        if (i.get_move()->to_string() == move.to_string()) {
            /* Apply matched move */
            current_position = *(i.get_result());
            update_next_moves();
            return i;
        }
    }

    throw std::runtime_error(std::string("Illegal move: ") + move.to_string());
}

void Game::update_next_moves() {
    legal_next_moves = current_position.get_legal_moves();
}

Position Game::get_current_position() {
    return current_position;
}

std::list<Position::Transition> Game::get_legal_next_moves() {
    return legal_next_moves;
}
