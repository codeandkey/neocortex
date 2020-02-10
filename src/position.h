#pragma once

#include <vector>
#include <utility>

#include "types.h"
#include "move.h"
#include "occ.h"
#include "eval_type.h"

namespace nc2 {
    class Position {
        public:
            typedef std::pair<Move, Position> Transition;

            /**
             * Constructs a new standard position.
             */
            Position(); /* standard */

            /**
             * Constructs a position from a FEN source.
             *
             * @param fen FEN input.
             */
            Position(std::string fen);

            /**
             * Generates legal moves for the position.
             *
             * @return Legal transition list.
             */
            std::vector<Transition> gen_legal_moves();

            /**
             * Gets a printable string with lots of info about the Position.
             *
             * @return Debug string.
             */
            std::string get_debug_string();

            /**
             * Gets the transposition table index for this position.
             *
             * @return Transposition table key.
             */
            u64 get_ttable_key();

            /**
             * Gets the evaluation heuristic for this position.
             * NOTE: this is NOT a search, but just the estimated position value.
             *
             * @return Eval heuristic.
             */
            float get_eval_heuristic();

            /**
             * Checks if the node is quiet.
             * Loud positions occur when a piece is captured or check is delivered.
             *
             * @return true if position is quiet, false otherwise.
             */
            bool is_quiet();

            /**
             * Gets the color to move.
             *
             * @return Color to move.
             */
            u8 get_color_to_move();

            /**
             * Tests if a color is in check.
             *
             * @param col Color to test.
             *
             * @return true if `col` is in check, false otherwise.
             */
            bool get_color_in_check(u8 col);

            /**
             * Gets a pointer to the board state.
             *
             * @return Board pointer.
             */
            u8* get_board();
            
        private:
            Occboard global_occ;
            Occboard color_occ[2];
            u8 color_to_move;
            u8 board[64];
            u8 en_passant_target;
            u64 ttable_index;

            u64 attack_masks[2]; /* [color] */
            u64 king_masks[2]; /* [color] */
            bool check_states[2]; /* [color] */

            bool quiet;

            /* FEN bits, game state */
            bool castle_states[2][2]; /* [color][kingside] */
            int fullmove_number, halfmove_clock;

            bool computed_eval;
            float current_eval;

            /**
             * Generates all pseudolegal moves.
             * Does not test if the resulting position has the king in check, this is done later in gen_legal_moves().
             *
             * @return List of pseudolegal transitions.
             */
            std::vector<Transition> gen_pseudolegal_moves();
            
            /**
             * Filters basic input moves for well-behaved pieces (rook, knight, bishop, king, queen).
             *
             * @param source Input move list.
             * @param out Destination move list.
             */
            void filter_basic_moves(const std::vector<Move>& source, std::vector<Transition>* out);

            /**
             * Filters valid pawn captures into a transition output.
             *
             * @param source Input move list.
             * @param out Destination move list.
             */
            void filter_pawn_captures(const std::vector<Move>& source, std::vector<Transition>* out);

            /**
             * Filters pawn advances.
             *
             * @param source Input move list.
             * @param out Destination move list.
             */
            void filter_pawn_advances(const std::vector<Move>& source, std::vector<Transition>* out);

            /**
             * Generates legal castling moves.
             *
             * @return Legal move list.
             */
            std::vector<Transition> gen_castle_moves();

            /**
             * Updates the position check states.
             * If the color not to move is currently in check then the position is illegal.
             *
             * @return true if the position is legal, false otherwise.
             */
            bool update_check_states();

            /**
             * Computes the eval heuristic if it needs an update.
             */
            void compute_eval();
    };
}
