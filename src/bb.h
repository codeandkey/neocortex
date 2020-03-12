#pragma once

/*
 * Generic bitboard type and manipulation funcs.
 */

#include <stdint.h>

#include "square.h"

typedef uint64_t nc_bb;

/* Useful bitboard literals */
#define NC_BB_RANK1 0x00000000000000FFULL
#define NC_BB_RANK(r) (NC_BB_RANK1 << (8*r))

#define NC_BB_RANK2 NC_BB_RANK(1)
#define NC_BB_RANK3 NC_BB_RANK(2)
#define NC_BB_RANK4 NC_BB_RANK(3)
#define NC_BB_RANK5 NC_BB_RANK(4)
#define NC_BB_RANK6 NC_BB_RANK(5)
#define NC_BB_RANK7 NC_BB_RANK(6)
#define NC_BB_RANK8 NC_BB_RANK(7)

#define NC_BB_FILEA 0x0101010101010101ULL
#define NC_BB_FILE(f) (NC_BB_FILEA << f)

#define NC_BB_FILEB NC_BB_FILE(1)
#define NC_BB_FILEC NC_BB_FILE(2)
#define NC_BB_FILED NC_BB_FILE(3)
#define NC_BB_FILEE NC_BB_FILE(4)
#define NC_BB_FILEF NC_BB_FILE(5)
#define NC_BB_FILEG NC_BB_FILE(6)
#define NC_BB_FILEH NC_BB_FILE(7)

#define NC_BB_QS_BADFILES (NC_BB_FILEC | NC_BB_FILED | NC_BB_FILEE)
#define NC_BB_KS_BADFILES (NC_BB_FILEE | NC_BB_FILEF | NC_BB_FILEG)
#define NC_BB_QS_OCCFILES (NC_BB_FILEB | NC_BB_FILEC | NC_BB_FILED)
#define NC_BB_KS_OCCFILES (NC_BB_FILEF | NC_BB_FILEG)

/* String conversion */
const char* nc_bb_tostr(nc_bb bb);

/* Inline manipulation */
static inline nc_bb nc_bb_shift(nc_bb bb, int dir) {
    return (dir > 0) ? (bb << dir) : (bb >> -dir);
}

static inline nc_bb nc_bb_mask(nc_square sq) {
    if (!nc_square_is_valid(sq)) return 0;
    return ((nc_bb) 1) << sq;
}

static inline nc_square nc_bb_getlsb(nc_bb bb) {
    nc_assert(bb);
    return __builtin_ctzll(bb);
}

static inline nc_square nc_bb_poplsb(nc_bb* bb) {
    nc_square lsb = nc_bb_getlsb(*bb);
    *bb ^= nc_bb_mask(lsb);
    return lsb;
}
