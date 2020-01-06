#pragma once

#include <string>

/**
 * Square type. Represents a square location on the board, or NULL.
 */

namespace nc {
    class Square {
    public:
        /**
         * Initializes a NULL square.
         */
        Square();

        /**
         * Initializes a square at a particular location.
         *
         * @param rank Square rank (1 -> 8 inclusive)
         * @param file Square file (1 -> 8 inclusive)
         */
        Square(int rank, int file);

        /**
         * Initializes a square from a UCI string.
         * Example: e4, c5, a1
         *
         * @param uci UCI square coordinates.
         */
        Square(std::string uci);

        /**
         * Moves the square in-place
         *
         * @param ranks Ranks to move.
         * @param files Files to move.
         */
        void shift(int ranks, int files);

        /**
         * Gets the square rank.
         *
         * @return rank from 1 to 8 or -1 if null.
         */
        int get_rank();

        /**
         * Gets the square file.
         *
         * @return rank from 1 to 8 or -1 if null.
         */
        int get_file();

        /**
         * Gets the array index for the square.
         * Returns -1 if null.
         *
         * @return Array index from 0 to 63 or -1 if null.
         */
        int get_index();

        /**
         * Returns true if the square is non-null.
         *
         * @return false if null, true otherwise.
         */
        bool is_valid();

        /**
         * Shorthand for is_valid()
         */
        operator bool();

        /**
         * Shorthand for get_index()
         */
        operator int();

    private:
        int index, rank, file;

        /**
         * Private: updates the index from the rank and file, and verifies that the coordinates make sense.
         */
        void update();
    };
}
