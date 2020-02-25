#pragma once

#include "util.h"

#include <ctype.h>

/*
 * Piece types and constants
 */

/* Piece types */
typedef int nc_ptype;

#define NC_PTYPE_NULL -1
#define NC_PAWN 0
#define NC_ROOK 1
#define NC_KNIGHT 2
#define NC_BISHOP 3
#define NC_QUEEN 4
#define NC_KING 5

/* Piece type manipulation */
static inline int nc_ptype_is_valid(nc_ptype pt) {
    return (pt >= 0 && pt <= 5);
}

static inline char nc_ptype_tochar(nc_ptype pt) {
    nc_assert(nc_ptype_is_valid(pt));
    return "prkbqk"[pt]; /* sneaky */
}

static inline nc_ptype nc_ptype_fromchar(char c) {
    switch (c) {
    case 'p':
        return NC_PAWN;
    case 'r':
        return NC_ROOK;
    case 'n':
        return NC_KNIGHT;
    case 'b':
        return NC_BISHOP;
    case 'q':
        return NC_QUEEN;
    case 'k':
        return NC_KING;
    default:
        return NC_PTYPE_NULL;
    }
}

/* Piece colors */
typedef int nc_color;

#define NC_WHITE 0
#define NC_BLACK 1

/* Color manipulation */
static inline int nc_color_is_valid(nc_color in) {
    return (in >= 0 && in <= 1);
}

static inline nc_color nc_colorflip(nc_color in) {
    nc_assert(nc_color_is_valid(in));
    return !in;
}

static inline char nc_colorchar(nc_color in) {
    nc_assert(nc_color_is_valid(in));
    return (in == NC_WHITE) ? 'w' : 'b';
}

/* Generic piece type. */
typedef int nc_piece;

/* Piece literals */
#define NC_PIECE_NULL -1
#define NC_WHITE_PAWN 0
#define NC_BLACK_PAWN 1
#define NC_WHITE_ROOK 2
#define NC_BLACK_ROOK 3
#define NC_WHITE_KNIGHT 4
#define NC_BLACK_KNIGHT 5
#define NC_WHITE_BISHOP 6
#define NC_BLACK_BISHOP 7
#define NC_WHITE_QUEEN 8
#define NC_BLACK_QUEEN 9
#define NC_WHITE_KING 10
#define NC_BLACK_KING 11

/* Piece manipulation */
static inline int nc_piece_is_valid(nc_piece p) {
    return (p >= 0 && p <= 11);
}

static inline nc_piece nc_piece_make(nc_color col, nc_ptype t) {
    nc_assert(nc_color_is_valid(col));
    nc_assert(nc_ptype_is_valid(t));

    return (t << 1) | col;
}

static inline nc_color nc_piece_color(nc_piece p) {
    nc_assert(nc_piece_is_valid(p));
    return p & 1;
}

static inline nc_ptype nc_pieceype(nc_piece p) {
    nc_assert(nc_piece_is_valid(p));
    return p >> 1;
}

static inline char nc_piece_touci(nc_piece p) {
    nc_assert(nc_piece_is_valid(p));
    return "PpRrNnBbQqKk"[p];
}

static inline nc_piece nc_piece_fromuci(char in) {
    char lc = tolower(in);
    nc_ptype pt = nc_ptype_fromchar(lc);

    if (!nc_ptype_is_valid(pt)) return NC_PIECE_NULL;
    return nc_piece_make((lc == in) ? NC_BLACK : NC_WHITE, pt);
}
