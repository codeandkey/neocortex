#pragma once

#include "platform.h"

#include <assert.h>
#include <stdint.h>

typedef uint64_t ncBitboard;
typedef int      ncColor;
typedef uint64_t ncHashKey;
typedef int      ncMove;
typedef int      ncPiece;
typedef int      ncSquare;

#define NC_RANK_1 0xFFULL
#define NC_RANK_2 (0xFFULL << 8)
#define NC_RANK_3 (0xFFULL << 16)
#define NC_RANK_4 (0xFFULL << 24)
#define NC_RANK_5 (0xFFULL << 32)
#define NC_RANK_6 (0xFFULL << 40)
#define NC_RANK_7 (0xFFULL << 48)
#define NC_RANK_8 (0xFFULL << 56)

#define NC_FILE_A 0x0101010101010101ULL
#define NC_FILE_B (NC_FILE_A << 1)
#define NC_FILE_C (NC_FILE_A << 2)
#define NC_FILE_D (NC_FILE_A << 3)
#define NC_FILE_E (NC_FILE_A << 4)
#define NC_FILE_F (NC_FILE_A << 5)
#define NC_FILE_G (NC_FILE_A << 6)
#define NC_FILE_H (NC_FILE_A << 7)

#define NC_EAST 1
#define NC_WEST -1
#define NC_NORTH 8
#define NC_SOUTH -8
#define NC_NORTHEAST 9
#define NC_NORTHWEST 7
#define NC_SOUTHEAST -7
#define NC_SOUTHWEST -9

#define NC_WHITE 0
#define NC_BLACK 1

#define NC_NULL -1
#define NC_PAWN 0
#define NC_KNIGHT 1
#define NC_BISHOP 2
#define NC_ROOK 3
#define NC_QUEEN 4
#define NC_KING 5

static inline int ncBitboardShift(ncBitboard b, int dir)
{
    return (dir > 0) ? b << dir : b >> -dir;
}

static inline int ncPieceValid(ncPiece p)
{
    return p >= 0 && p < 12;
}

static inline int ncPieceTypeValid(ncPiece p)
{
    return p >= 0 && p < 6;
}

static inline ncColor ncPieceColor(ncPiece p)
{
    assert(ncPieceValid(p));
    return p & 1;
}

static inline ncPiece ncPieceFromChar(char c)
{
    switch (c)
    {
        case 'p':
            return NC_BLACK | (NC_PAWN << 1);
        default:
            return NC_NULL;
    }
}

static inline ncPiece ncPieceType(ncPiece p)
{
    assert(ncPieceValid(p));
    return p >> 1;
}

static inline ncSquare ncSquareAt(int rank, int file)
{
    assert(rank >= 0 && rank < 8);
    assert(file >= 0 && file < 8);

    return rank * 8 + file;
}

static inline int ncSquareValid(ncSquare s) { return s >= 0 & s < 64; }

static inline int ncSquareFile(ncSquare s)
{
    assert(ncSquareValid(s));
    return s & 0x3;
}

static inline int ncSquareRank(ncSquare s)
{
    assert(ncSquareValid(s));
    return s / 8;
}

static inline ncBitboard ncSquareMask(ncSquare s)
{
    assert(ncSquareValid(s));
    return 1ULL << s;
}
