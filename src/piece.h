#pragma once

/**
 * Defines a game piece, or a NULL piece (empty square).
 */

namespace nc {
    class Piece {
    public:
        /**
         * Default constructor. Initializes a NULL piece.
         */
        Piece(); /* null piece */

        /**
         * UCI constructor. Accepts a valid UCI piece character.
         * Valid uci: K, Q, B, N, R, P, k, q, b, n, r, p
         * 
         * @param uci UCI code
         */
        Piece(char uci);

        /**
         * Initializes a piece from a color and type.
         *
         * @param col Color ('w' or 'b')
         * @param type Type ('k', 'q', 'b', 'n', 'r', or 'p')
         */
        Piece(char col, char type);

        /**
         * Updates the piece from the color and type.
         *
         * @param col Color
         * @param type Type
         */
        void set(char col, char type);

        /**
         * Get the color for the piece, or 0 if the piece is null.
         *
         * @return 'w', 'b', or 0.
         */
        char get_color();

        /**
         * Gets the lowercase type for the piece.
         * If the piece is null returns 0.
         *
         * @return Type char ('k', 'q', 'b', 'n', 'r', 'p') or '\0'
         */
        char get_type();

        /**
         * Gets a UCI-compatible piece code for the piece.
         * If the piece is null, this returns 0.
         *
         * @return UCI code, or '\0'
         */
        char get_uci();

        /**
         * Returns true if the piece is a valid game piece.
         *
         * @return True if piece is a piece, false otherwise.
         */
        bool is_valid();

        /**
         * Shorthand for get_uci().
         */
        operator char();

        /**
         * Shorthand for is_valid().
         */
        operator bool();
    private:
        char uci, col, type;
    };
}
