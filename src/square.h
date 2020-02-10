#pragma once

/*
 * Square utilities
 */

#include <string>

#include "types.h"

namespace nc2 {
    namespace square {
        constexpr u8 null = 0xFF;

        /**
         * Gets a square at a location. r and f should be between 0 and 7 inclusive.
         * 
         * @param r Rank
         * @param f File
         *
         * @return Target square.
         */
        u8 at(u8 r, u8 f);

        /**
         * Gets a square at a location. or returns square::null if the coordinates are out of bounds.
         * 
         * @param r Rank
         * @param f File
         *
         * @return Target square, or square::null.
         */
        u8 at_safe(u8 r, u8 f);

        /**
         * Gets the rank of a square.
         * 
         * @param s Square
         *
         * @return Square rank, from 0 to 7 inclusive.
         */
        u8 rank(u8 s);

        /**
         * Gets the file of a square.
         * 
         * @param s Square
         *
         * @return Square file, from 0 to 7 inclusive.
         */
        u8 file(u8 s);

        /**
         * Gets the diagonal of a square.
         * 
         * @param s Square
         *
         * @return Square diagonal, from 0 to 14 inclusive.
         */
        u8 diag(u8 s);

        /**
         * Gets the antidiagonal of a square.
         * 
         * @param s Square
         *
         * @return Square antidiagonal, from 0 to 14 inclusive.
         */
        u8 antidiag(u8 s);

        /**
         * Shifts a square by some number of ranks and files.
         * 
         * @param s Square
         * @param r Ranks
         * @param f Files
         *
         * @return New square.
         */
        u8 shift(u8 s, int r, int f);

        /**
         * Gets the bitboard mask for a single square.
         *
         * @param s Square (0 <= s < 64)
         *
         * @return Bitboard mask.
         */
        u64 mask(u8 s);

        /**
         * Gets the square as a UCI string.
         *
         * @param s Square
         *
         * @return String representation.
         */
        std::string to_string(u8 s);

        enum Squares {
            a1, b1, c1, d1, e1, f1, g1, h1,
            a2, b2, c2, d2, e2, f2, g2, h2,
            a3, b3, c3, d3, e3, f3, g3, h3,
            a4, b4, c4, d4, e4, f4, g4, h4,
            a5, b5, c5, d5, e5, f5, g5, h5,
            a6, b6, c6, d6, e6, f6, g6, h6,
            a7, b7, c7, d7, e7, f7, g7, h7,
            a8, b8, c8, d8, e8, f8, g8, h8,
        };

        /* constant square masks, used for building constant mask values */
        constexpr u64 MASK_A1 = ((u64) 1 << 0);
        constexpr u64 MASK_B1 = ((u64) 1 << 1);
        constexpr u64 MASK_C1 = ((u64) 1 << 2);
        constexpr u64 MASK_D1 = ((u64) 1 << 3);
        constexpr u64 MASK_E1 = ((u64) 1 << 4);
        constexpr u64 MASK_F1 = ((u64) 1 << 5);
        constexpr u64 MASK_G1 = ((u64) 1 << 6);
        constexpr u64 MASK_H1 = ((u64) 1 << 7);
        constexpr u64 MASK_A2 = ((u64) 1 << 8);
        constexpr u64 MASK_B2 = ((u64) 1 << 9);
        constexpr u64 MASK_C2 = ((u64) 1 << 10);
        constexpr u64 MASK_D2 = ((u64) 1 << 11);
        constexpr u64 MASK_E2 = ((u64) 1 << 12);
        constexpr u64 MASK_F2 = ((u64) 1 << 13);
        constexpr u64 MASK_G2 = ((u64) 1 << 14);
        constexpr u64 MASK_H2 = ((u64) 1 << 15);
        constexpr u64 MASK_A3 = ((u64) 1 << 16);
        constexpr u64 MASK_B3 = ((u64) 1 << 17);
        constexpr u64 MASK_C3 = ((u64) 1 << 18);
        constexpr u64 MASK_D3 = ((u64) 1 << 19);
        constexpr u64 MASK_E3 = ((u64) 1 << 20);
        constexpr u64 MASK_F3 = ((u64) 1 << 21);
        constexpr u64 MASK_G3 = ((u64) 1 << 22);
        constexpr u64 MASK_H3 = ((u64) 1 << 23);
        constexpr u64 MASK_A4 = ((u64) 1 << 24);
        constexpr u64 MASK_B4 = ((u64) 1 << 25);
        constexpr u64 MASK_C4 = ((u64) 1 << 26);
        constexpr u64 MASK_D4 = ((u64) 1 << 27);
        constexpr u64 MASK_E4 = ((u64) 1 << 28);
        constexpr u64 MASK_F4 = ((u64) 1 << 29);
        constexpr u64 MASK_G4 = ((u64) 1 << 30);
        constexpr u64 MASK_H4 = ((u64) 1 << 31);
        constexpr u64 MASK_A5 = ((u64) 1 << 32);
        constexpr u64 MASK_B5 = ((u64) 1 << 33);
        constexpr u64 MASK_C5 = ((u64) 1 << 34);
        constexpr u64 MASK_D5 = ((u64) 1 << 35);
        constexpr u64 MASK_E5 = ((u64) 1 << 36);
        constexpr u64 MASK_F5 = ((u64) 1 << 37);
        constexpr u64 MASK_G5 = ((u64) 1 << 38);
        constexpr u64 MASK_H5 = ((u64) 1 << 39);
        constexpr u64 MASK_A6 = ((u64) 1 << 40);
        constexpr u64 MASK_B6 = ((u64) 1 << 41);
        constexpr u64 MASK_C6 = ((u64) 1 << 42);
        constexpr u64 MASK_D6 = ((u64) 1 << 43);
        constexpr u64 MASK_E6 = ((u64) 1 << 44);
        constexpr u64 MASK_F6 = ((u64) 1 << 45);
        constexpr u64 MASK_G6 = ((u64) 1 << 46);
        constexpr u64 MASK_H6 = ((u64) 1 << 47);
        constexpr u64 MASK_A7 = ((u64) 1 << 48);
        constexpr u64 MASK_B7 = ((u64) 1 << 49);
        constexpr u64 MASK_C7 = ((u64) 1 << 50);
        constexpr u64 MASK_D7 = ((u64) 1 << 51);
        constexpr u64 MASK_E7 = ((u64) 1 << 52);
        constexpr u64 MASK_F7 = ((u64) 1 << 53);
        constexpr u64 MASK_G7 = ((u64) 1 << 54);
        constexpr u64 MASK_H7 = ((u64) 1 << 55);
        constexpr u64 MASK_A8 = ((u64) 1 << 56);
        constexpr u64 MASK_B8 = ((u64) 1 << 57);
        constexpr u64 MASK_C8 = ((u64) 1 << 58);
        constexpr u64 MASK_D8 = ((u64) 1 << 59);
        constexpr u64 MASK_E8 = ((u64) 1 << 60);
        constexpr u64 MASK_F8 = ((u64) 1 << 61);
        constexpr u64 MASK_G8 = ((u64) 1 << 62);
        constexpr u64 MASK_H8 = ((u64) 1 << 63);
    }
}
