#pragma once

#include <string>

#include "position.h"

namespace nc {
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
         */
        void apply(Move move);

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
        std::list<std::pair<Move, Position>> get_legal_next_moves();

    private:
        Position current_position;
        std::list<std::pair<Move, Position>> legal_next_moves;

        /**
         * Updates the internal list of next legal moves.
         */
        void update_next_moves();
    };
}
