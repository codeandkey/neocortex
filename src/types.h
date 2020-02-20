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

#define NC3_SQNULL (-1)
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

#define NC3_PNULL 0
#define NC3_PAWN 1
#define NC3_BISHOP 2
#define NC3_ROOK 3
#define NC3_KNIGHT 4
#define NC3_QUEEN 5
#define NC3_KING 6

#define NC3_BLACK 0
#define NC3_WHITE 1

#define NC3_EVAL_MUL(col) (col * 2 - 1)
#define NC3_TYPE(p) (p & 7)
#define NC3_COLOR(p) ((p >> 3) & 1)

/*
 * Move type.
 *
 * bits [0, 5]: destination square
 * bits [6, 10]: source square
 * bits [11, 13]: promote type
 * bit 14 : null flag
 */

typedef int nc3_move;

#define NC3_MVNULL 0
#define NC3_MOVE(src, dst) ((src << 6) | dst)
#define NC3_PMOVE(src, dst, ptype) (NC3_MOVE(src, dst) | (ptype << 12))
#define NC3_PROMOTE 0xF000

#define NC3_GETSRC(mv) ((mv >> 6) & 0x3F)
#define NC3_GETDST(mv) (mv & 0x3F)
#define NC3_GETPTYPE(mv) ((mv >> 12) & 0x3F)

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
