#include "game.h"

#include <stdexcept>

using namespace nc2;

Game::Game() {
    update_next_moves();
}

Game::Game(std::string fen) : current_position(fen) {
    update_next_moves();
}

Position::Transition Game::apply(std::string move) {
    for (auto i : legal_next_moves) {
        if (i.first.to_string() == move) {
            /* Apply matched move */
            current_position = i.second;
            update_next_moves();
            return i;
        }
    }

    throw std::runtime_error(std::string("Illegal move: ") + move);
}

void Game::update_next_moves() {
    legal_next_moves = current_position.gen_legal_moves();
}

Position Game::get_current_position() {
    return current_position;
}

std::vector<Position::Transition> Game::get_legal_next_moves() {
    return legal_next_moves;
}
