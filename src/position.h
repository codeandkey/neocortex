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

        /**
         * Gets a mask of squares attacked by a color.
         *
         * @param col
         * @return Board attack mask
         */
        u64 get_color_attack_mask(char col);

        /**
         * Gets the heuristic evaluation of this position.
         * Does NOT perform a move search, that is done in Search.
         *
         * @return Evaluation
         */
        float get_eval();

        /**
         * Gets the color to move.
         *
         * @return 'w' or 'b'
         */
        char get_color_to_move();

        /**
         * Gets if the position is "quiet" (no color is in check)
         *
         * TODO: this should be extended -- captures should also be considered not quiet.
         * Perhaps the quiet property could be moved to the Move structure.
         *
         * @return true if position is quiet, false otherwise
         */
        bool is_quiet();

    protected:
        /* Some static board eval consts. */
        static constexpr float MAT_PAWN = 1.0f;
        static constexpr float MAT_BISHOP = 3.0f;
        static constexpr float MAT_KNIGHT = 3.0f;
        static constexpr float MAT_ROOK = 5.0f;
        static constexpr float MAT_QUEEN = 7.0f;

        /* Keep const non-pawn material total. */
        static constexpr float NPM_TOTAL = (4 * MAT_BISHOP) +
                                           (4 * MAT_KNIGHT) +
                                           (4 * MAT_ROOK) +
                                           (2 * MAT_QUEEN);

        bool white_in_check, black_in_check;
        bool w_kingside, w_queenside, b_kingside, b_queenside;
        Square en_passant_target;
        char color_to_move;
        Piece board[64];
        int halfmove_clock, fullmove_number;
        Occtable occ;
        u64 white_king_mask, black_king_mask;
        u64 white_attack_mask, black_attack_mask;

        /**
         * Generates pseudolegal moves and returns a list of ordered pairs,
         * of moves and their resulting positions.
         *
         * @return Pseudolegal moves.
         */
        std::list<Transition> get_pseudolegal_moves();

        /**
         * Generates legal castling moves.
         *
         * @param out List to add moves to.
         */
        void get_castle_moves(std::list<Transition>* out);

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

        /* -- Evaluation helpers -- */

        /**
         * Inverts a color.
         *
         * @param c Color to invert
         * @return Inverted color
         */
        char colorflip(char c);

        /**
         * Gets the evaluation worth of a color's center control.
         * Always positive.
         *
         * @param c Color to check.
         * @return Evaluation value.
         */
        float eval_center(char c);

        /**
         * Gets the evaluation worth of a color's development.
         * Always positive.
         *
         * @param c Color to check.
         * @return Evaluation value.
         */
        float eval_development(char c);

        /**
         * Gets the evaluation worth of a color's material.
         * Always positive.
         *
         * @param c Color to check.
         * @return Evaluation value.
         */
        float eval_material(char c);

        /**
         * Gets the game phase, where 0 => opening and 1 => endgame
         *
         * @return Phase value
         */
        float eval_get_phase();

        /**
         * Gets the material value for a piece type.
         *
         * @param t Type
         * @return Positive material value
         */
        float eval_get_piece_value(char t);

        /**
         * Computes attack masks for white and black, and updates white_in_check
         * and black_in_check accordingly.
         *
         * Also updates the white_attack_mask and black_attack_mask members.
         */
        void compute_attack_masks();
    };
}
