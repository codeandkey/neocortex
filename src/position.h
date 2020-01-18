#pragma once

#include "piece.h"
#include "square.h"
#include "occtable.h"
#include "move.h"

#include <string>
#include <list>
#include <utility>

namespace nc {
    class Position {
    public:
        /*
         * Internal class for full moves (a move and resulting position)
         */
        class Transition {
        public:
            /**
             * Initializes a transition. Assigns a move and resulting position.
             *
             * @param move Move
             * @param result Position after move is executed
             * @param check True iff the move delivers check to the other color
             * @param mate True iff the move delivers checkmate to the other color
             */
            Transition(Move move, Position result, bool check = false, bool mate = false);

            /**
             * Copy constructor.
             *
             * @param b Other transition
             */
            Transition(const Transition& b);

            /**
             * Destructor.
             */
            ~Transition();

            /**
             * Gets the resulting position associated with this transition.
             * 
             * @return Resulting position.
             */
            Position* get_result();

            /**
             * Gets the move associated with this transition.
             *
             * @return Move
             */
            Move* get_move();

            /**
             * Gets whether the move delivers check to the other color.
             *
             * @return true iff move delivers check, false otherwise
             */
            bool get_check();

            /**
             * Gets whether the move delivers checkmate to the other color.
             *
             * @return true iff move delivers checkmate, false otherwise
             */
            bool get_mate();

            /**
             * Sets the check flag on this transition.
             *
             * @param check Value to set.
             */
            void set_check(bool check);

            /**
             * Sets the checkmate flag on this transition.
             *
             * @param check Value to set.
             */
            void set_mate(bool mate);

            /**
             * Gets the transition in string form.
             * Returns the UCI move along with '+' and '#' flags for check and mate respectively.
             *
             * @return String representation.
             */
            std::string to_string();

            /**
             * Shorthand for to_string().
             */
            operator std::string();

        private:
            Move move;
            Position *result;
            bool check, mate;
        };

        /**
         * Initializes a standard initial position, with white to move.
         */
        Position();

        /**
         * Initializes a position from a FEN source.
         *
         * @param fen FEN source
         */
        Position(std::string fen);

        /**
         * Gets the position in FEN format.
         *
         * @return FEN string
         */
        std::string get_fen();

        /**
         * Shorthand for get_fen().
         */
        operator std::string();

        /**
         * Generates legal moves and returns a list of ordered pairs,
         * of moves and their resulting positions.
         *
         * @return Pseudolegal moves.
         */
        std::list<Transition> get_legal_moves();

        /**
         * Determine if a color is in check.
         *
         * @param col Color to test.
         * @return true iff <col> is in check, false otherwise.
         */
        bool get_color_in_check(char col);

    protected:
        bool white_in_check, black_in_check;
        bool w_kingside, w_queenside, b_kingside, b_queenside;
        Square en_passant_target;
        char color_to_move;
        Piece board[64];
        int halfmove_clock, fullmove_number;
        Occtable occ;

        /**
         * Generates pseudolegal moves and returns a list of ordered pairs,
         * of moves and their resulting positions.
         *
         * @return Pseudolegal moves.
         */
        std::list<Transition> get_pseudolegal_moves();

        /**
         * Generates pseudolegal rook moves from a square.
         * Assumes that the rook is the correct color (color_to_move).
         *
         * @param sq Origin square.
         * @param out List to append to.
         */
        void get_pseudolegal_rook_moves(Square sq, std::list<Transition>* out);

        /**
         * Generates pseudolegal bishop moves from a square.
         * Assumes that the bishop is the correct color (color_to_move).
         *
         * @param sq Origin square.
         * @param out List to append to.
         */
        void get_pseudolegal_bishop_moves(Square sq, std::list<Transition>* out);

        /**
         * Generates a basic piece move or capture. Appropriate for all moves except castling.
         *
         * @param from From square.
         * @param to To square.
         *
         * @return Pseudolegal move.
         */
        Transition make_basic_pseudolegal_move(Square from, Square to, char promote_type = '\0');
    };
}
