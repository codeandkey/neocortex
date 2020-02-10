#pragma once

/*
 * Piece constants and helpers.
 */

#include <vector>
#include "types.h"

namespace nc2 {
    namespace piece {
        enum Type {
            PAWN   = 0,
            KNIGHT = 1,
            BISHOP = 2,
            ROOK   = 3,
            KING   = 4,
            QUEEN  = 5,
            NONE   = 7,
        };

        enum Color {
            WHITE = 0,
            BLACK = 1,
        };

        constexpr u8 null = 0xF;

        constexpr u8 WHITE_PAWN = 0;
        constexpr u8 BLACK_PAWN = 1;
        constexpr u8 WHITE_KNIGHT= 2;
        constexpr u8 BLACK_KNIGHT = 3;
        constexpr u8 WHITE_BISHOP = 4;
        constexpr u8 BLACK_BISHOP = 5;
        constexpr u8 WHITE_ROOK = 6;
        constexpr u8 BLACK_ROOK = 7;
        constexpr u8 WHITE_KING = 8;
        constexpr u8 BLACK_KING = 9;
        constexpr u8 WHITE_QUEEN = 10;
        constexpr u8 BLACK_QUEEN = 11;

        /**
         * Makes a piece from a type and color.
         *
         * @param type Piece type.
         * @param color Piece color.
         *
         * @return New piece value.
         */
        u8 make(u8 type, u8 color);

        /**
         * Gets the type of a piece.
         *
         * @param p Piece value.
         * @return Piece type.
         */
        u8 type(u8 p);

        /**
         * Gets the color of a piece.
         *
         * @param p Piece value.
         * @return Piece type.
         */
        u8 color(u8 p);

        /**
         * Gets the inverse of a color.
         *
         * @param c Input color.
         * @return Flipped color.
         */
        u8 colorflip(u8 c);

        /**
         * Gets a printable UCI character for a piece.
         *
         * @param p Input piece.
         * @return UCI character.
         */
        char uci(u8 p);

        /**
         * Gets a printable type character for a piece. Always lowercase.
         *
         * @param p Input piece.
         * @return Type character.
         */
        char type_char(u8 t);

        /**
         * Tests if a piece exists and is not null.
         *
         * @param p Input piece.
         * @return true if p is a piece, false otherwise.
         */
        bool exists(u8 p);

        /**
         * Initializes a vector of pieces from a FEN piece character.
         * '8' will translate to 8 empty squares, otherwise the vector will be of size 1 and contain the UCI
         * equivalent.
         *
         * @param FEN input character
         * @return List of pieces.
         */
        std::vector<u8> from_uci(char c);
    }
}
