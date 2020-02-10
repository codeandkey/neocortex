#pragma once

/*
 * Transposition table indexing.
 */

#include "types.h"

namespace nc2 {
    namespace ttable {
        /**
         * Initialize the transposition table indices.
         * RNG is first seeded with <seed>.
         *
         * @param seed RNG seed.
         */
        void initialize_indices(u32 seed);

        /**
         * Gets a transposition table key for a piece on a square.
         *
         * @param s Square
         * @param p Piece
         *
         * @return Transposition key.
         */
        u64 get_piece_key(u8 s, u8 p);

        /**
         * Gets a transposition table key for a castling state.
         *
         * @param color Color
         * @param side Kingside state
         *
         * @return Transposition key.
         */
        u64 get_castle_key(u8 color, u8 side);

        /**
         * Gets a transposition table key for black to move.
         *
         * @return Transposition key.
         */
        u64 get_black_to_move_key();

        /**
         * Gets a transposition table key for an en passant file.
         *
         * @param file En passant file.
         *
         * @return Transposition key.
         */
        u64 get_en_passant_file_key(u8 file);
    }
}
