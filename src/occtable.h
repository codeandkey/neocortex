#pragma once

/**
 * Occtable bitboards.
 */

#include "types.h"
#include "square.h"

namespace nc {
    class Occtable {
    public:
        Occtable();

        /**
         * Initialize an occupancy table with a standard board state.
         *
         * @return Occtable
         */
        static Occtable standard();

        /**
         * Flips a square on an occtable.
         *
         * @param s Square to flip
         */
        void flip(Square s);

        /**
         * Gets relevant occupancy for a rank.
         *
         * @param r Rank to get.
         * @return Relevant occupancy byte.
         */
        u8 get_rank(int r);

        /**
         * Gets relevant occupancy for a file.
         *
         * @param r Rank to get.
         * @return Relevant occupancy byte.
         */
        u8 get_file(int f);

        /**
         * Gets relevant occupancy for a diagonal.
         *
         * @param r Rank to get.
         * @return Relevant occupancy byte.
         */
        u8 get_diag(int d);

        /**
         * Gets relevant occupancy for an antidiagonal.
         *
         * @param r Rank to get.
         * @return Relevant occupancy byte.
         */
        u8 get_antidiag(int d);

    private:
        u8 ranks[8], files[8], diags[15], antidiags[15];
    };
}
