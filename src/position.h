#pragma once

#include <vector>
#include <utility>

#include "types.h"
#include "move.h"
#include "occ.h"

namespace nc2 {
    class Position {
        public:
            Position(); /* standard */
            Position(std::string fen);

            typedef std::pair<Move, Position> Transition;

            /*class Transition {
                public:
                    Transition(Position* current, Move m, Position result);

                private:
                    Move move;
                    Position result;
            };*/

            std::vector<Transition> gen_legal_moves();

            std::string get_debug_string();

            u32 get_ttable_key();

            float get_eval();

            bool is_quiet();
            u8 get_color_to_move();
            bool get_color_in_check(u8 col);
            
        private:
            Occboard global_occ;
            Occboard color_occ[2];
            u8 color_to_move;
            u8 board[64];
            u8 en_passant_target;
            u32 ttable_index;

            u64 attack_masks[2]; /* [color] */
            u64 king_masks[2]; /* [color] */
            bool check_states[2]; /* [color] */

            bool quiet;

            /* FEN bits, game state */
            bool castle_states[2][2]; /* [color][kingside] */
            int fullmove_number, halfmove_clock;

            /**
             * Generates all pseudolegal moves.
             * Does not test if the resulting position has the king in check, this is done later in gen_legal_moves().
             */
            std::vector<Transition> gen_pseudolegal_moves();
            
            /**
             * Filters basic input moves for well-behaved pieces (rook, knight, bishop, king, queen).
             */
            void filter_basic_moves(const std::vector<Move>& source, std::vector<Transition>* out);

            /**
             * Filters valid pawn captures into a transition output.
             */
            void filter_pawn_captures(const std::vector<Move>& source, std::vector<Transition>* out);

            /**
             * Filters pawn advances.
             */
            void filter_pawn_advances(const std::vector<Move>& source, std::vector<Transition>* out);

            /**
             * Generates legal castling moves.
             */
            std::vector<Transition> gen_castle_moves();

            /**
             * Updates the position check states.
             * If the color not to move is currently in check then the position is illegal.
             *
             * Returns true if the position is legal, false otherwise.
             */
            bool update_check_states();
    };
}
