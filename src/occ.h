#pragma once

/*
 * Occupancy bitboard.
 */

#include "types.h"

namespace nc2 {
    class Occboard {
        public:
            /**
             * Initializes an empty occboard.
             */
            Occboard();

            /**
             * Initializes a standard occboard.
             *
             * @return Occboard for standard game.
             */
            static Occboard standard();

            /**
             * Flips a square in the board.
             *
             * @param s Square to flip.
             */
            void flip(u8 s);

            /**
             * Gets the occupancy mask for the whole board.
             *
             * @return Occupancy bitboard.
             */
            u64 get_board();

            /**
             * Gets the occupancy of a single rank.
             *
             * @param r Rank to get.
             * @return Rank occupancy.
             */
            u8 get_rank(u8 r);

            /**
             * Gets the occupancy of a single file.
             *
             * @param f File to get.
             * @return File occupancy.
             */
            u8 get_file(u8 f);

            /**
             * Gets the occupancy of a single diagonal.
             *
             * @param d Diagonal to get.
             * @return Diagonal occupancy.
             */
            u8 get_diag(u8 d);

            /**
             * Gets the occupancy of a single antidiagonal.
             *
             * @param d Antidiagonal to get.
             * @return Antidiagonal occupancy.
             */
            u8 get_antidiag(u8 d);

            /**
             * Tests if a pawn can legally jump by occupancy. This is necessary but not sufficient.
             *
             * @param f File to test.
             * @param col Color to test.
             * @return true if pawn can jump, false otherwise.
             */
            bool pawn_can_jump(u8 f, u8 col);

            /**
             * Tests if a color can castle (occupancy wise). This is necessary but not sufficient.
             *
             * @param col Color to test.
             * @param is_kingside Side to test.
             *
             * @return true if color can castle on `is_kingside`, false otherwise.
             */
            bool color_can_castle(u8 col, int is_kingside);

            /**
             * Tests if a single square is set in the bitboard.
             *
             * @param s Square to test.
             *
             * @return true if s is occupied, false otherwise.
             */
            bool test(u8 s);

            /**
             * Tests if any squares in a mask are set.
             *
             * @param m Mask to test.
             *
             * @return true if any squares in `m` are occupied, false otherwise.
             */
            bool test_mask(u64 m);

            /**
             * Converts an occupancy to a relevant occupancy (inner 6 bits).
             *
             * @param occ_byte Input occupancy.
             * @return Relevant occupancy.
             */
            static u8 to_rocc(u8 occ_byte);
        private:
            u8 diags[15], antidiags[15], ranks[8], files[8];
            u64 board;
    };
}
