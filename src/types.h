#pragma once

#include <stdint.h>

/*
 * Standardized width integer types.
 */

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

/*
 * Square type.
 */

typedef int nc3_square;

#define NC3_NULL -1

#define NC3_SQNULL NC3_NULL
#define NC3_SQMASK(s) (1ULL << s)
#define NC3_SQAT(r, f) (r * 8 + f)
#define NC3_RANK(sq) (sq >> 3)
#define NC3_FILE(sq) (sq & 7)

nc3_square nc3_square_fromstr(const char* inp);
const char* nc3_square_tostr(nc3_square sq);

/*
 * Piece type.
 *
 * bits [0, 2] : type
 * bit  [3]    : color
 */

#define NC3_PNULL NC3_NULL

#define NC3_PAWN 0
#define NC3_BISHOP 1
#define NC3_ROOK 2
#define NC3_KNIGHT 3
#define NC3_QUEEN 4
#define NC3_KING 5

#define NC3_WHITE 0
#define NC3_BLACK 1

#define NC3_WHITE_PAWN 0
#define NC3_WHITE_BISHOP 2
#define NC3_WHITE_ROOK 4
#define NC3_WHITE_KNIGHT 6
#define NC3_WHITE_QUEEN 8
#define NC3_WHITE_KING 10

#define NC3_BLACK_PAWN 1
#define NC3_BLACK_BISHOP 3
#define NC3_BLACK_ROOK 5
#define NC3_BLACK_KNIGHT 7
#define NC3_BLACK_QUEEN 9
#define NC3_BLACK_KING 11

#define NC3_EVAL_MUL(col) (-col * 2 + 1)
#define NC3_TYPE(p) (p & 7)
#define NC3_COLOR(p) ((p >> 3) & 1)

/*
 * Move type.
 *
 * low 3 nibbles : src, dst squares
 * 4th nibble: flags
 * 5th nibble: captured piece code
 *
 * bits [0, 5]: destination square
 * bits [6, 11]: source square
 * bits [12, 15]: flags
 */

typedef int nc3_move;

#define NC3_MVNULL NC3_NULL

#define NC3_MOVE(src, dst) ((src << 6) | dst)

/* Move modifier flags */
#define NC3_PROMOTION  0x1000
#define NC3_CAPTURE    0x2000
#define NC3_DOUBLEPUSH 0x4000

#define NC3_PROMOTION_TO(type) (NC3_PROMOTION | (type << 16))

#define NC3_GETSRC(mv) ((mv >> 6) & 0x3F)
#define NC3_GETDST(mv) (mv & 0x3F)
#define NC3_GETPTYPE(mv) ((mv >> 16) & 7)

const char* nc3_move_tostr(nc3_move mv);
nc3_move nc3_move_fromstr(const char* inp);

typedef struct {
    nc3_move* moves;
    int count;
} nc3_movelist;

void nc3_movelist_init(nc3_movelist* dst, int count);
void nc3_movelist_free(nc3_movelist* dst);
void nc3_movelist_add(nc3_movelist* dst, nc3_move move);

/*
 * Bitboard helpers.
 */

const char* nc3_bitboard_tostr(u64 bb);
