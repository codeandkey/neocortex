#pragma once

/**
 * Bitboard lookup table.
 *
 * Used for quickly generating attacked square masks.
 */

#include "types.h"
#include "square.h"
#include "occtable.h"

namespace nc {
    namespace lookup {
        /**
         * Initializes the lookup tables.
         * Must be called before any lookups are performed, otherwise the returned values will be invalid (0).
         */
        void init();

        /**
         * Looks up a sliding piece attack mask.
         *
         * @param pos Origin position.
         * @param rocc Relevant occupancy.
         *
         * @return Attack mask.
         */
        u8 sliding_attack(u8 pos, u8 rocc);

        /**
         * Looks up a sliding rank attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 rank_attack(Square s, Occtable* occ);

        /**
         * Looks up a sliding file attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 file_attack(Square s, Occtable* occ);

        /**
         * Looks up a sliding diagonal attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 diag_attack(Square s, Occtable* occ);

        /**
         * Looks up a sliding antidiagonal attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 antidiag_attack(Square s, Occtable* occ);

        /**
         * Looks up a rook attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 rook_attack(Square s, Occtable* occ);

        /**
         * Looks up a bishop attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 bishop_attack(Square s, Occtable* occ);

        /**
         * Looks up a queen attack mask.
         *
         * @param s Origin square.
         * @param occ Occupancy table.
         *
         * @return Board attack mask.
         */
        u64 queen_attack(Square s, Occtable* occ);
    }
}
