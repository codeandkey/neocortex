#pragma once

#include <string>

#include "position.h"
#include "move.h"

namespace nc2 {
    class Game {
    public:
        /**
         * Starts a standard game.
         */
        Game();

        /**
         * Starts a game from a FEN.
         *
         * @param fen FEN string
         */
        Game(std::string fen);

        /**
         * Applies a move.
         * Raises a runtime exception if the move is illegal.
         *
         * @param move Move to apply.
         * @return Applied move transition.
         */
        Position::Transition apply(std::string move);

        /**
         * Gets the current game position.
         *
         * @return Current game state.
         */
        Position get_current_position();

        /**
         * Gets the list of legal next moves.
         *
         * @return list of legal transitions.
         */
        std::vector<Position::Transition> get_legal_next_moves();

    private:
        Position current_position;
        std::vector<Position::Transition> legal_next_moves;

        /**
         * Updates the internal list of next legal moves.
         */
        void update_next_moves();
    };
}
