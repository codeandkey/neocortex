#pragma once

/*
 * Square type.
 */

#include "util.h"

typedef int nc_square;

/* Square literals */
#define NC_SQ_NULL -1

#define NC_SQ_A1 0
#define NC_SQ_B1 1
#define NC_SQ_C1 2
#define NC_SQ_D1 3
#define NC_SQ_E1 4
#define NC_SQ_F1 5
#define NC_SQ_G1 6
#define NC_SQ_H1 7

#define NC_SQ_A2 8
#define NC_SQ_B2 9
#define NC_SQ_C2 10
#define NC_SQ_D2 11
#define NC_SQ_E2 12
#define NC_SQ_F2 13
#define NC_SQ_G2 14
#define NC_SQ_H2 15

#define NC_SQ_A3 16
#define NC_SQ_B3 17
#define NC_SQ_C3 18
#define NC_SQ_D3 19
#define NC_SQ_E3 20
#define NC_SQ_F3 21
#define NC_SQ_G3 22
#define NC_SQ_H3 23

#define NC_SQ_A4 24
#define NC_SQ_B4 25
#define NC_SQ_C4 26
#define NC_SQ_D4 27
#define NC_SQ_E4 28
#define NC_SQ_F4 29
#define NC_SQ_G4 30
#define NC_SQ_H4 31

#define NC_SQ_A5 32
#define NC_SQ_B5 33
#define NC_SQ_C5 34
#define NC_SQ_D5 35
#define NC_SQ_E5 36
#define NC_SQ_F5 37
#define NC_SQ_G5 38
#define NC_SQ_H5 39

#define NC_SQ_A6 40
#define NC_SQ_B6 41
#define NC_SQ_C6 42
#define NC_SQ_D6 43
#define NC_SQ_E6 44
#define NC_SQ_F6 45
#define NC_SQ_G6 46
#define NC_SQ_H6 47

#define NC_SQ_A7 48
#define NC_SQ_B7 49
#define NC_SQ_C7 50
#define NC_SQ_D7 51
#define NC_SQ_E7 52
#define NC_SQ_F7 53
#define NC_SQ_G7 54
#define NC_SQ_H7 55

#define NC_SQ_A8 56
#define NC_SQ_B8 57
#define NC_SQ_C8 58
#define NC_SQ_D8 59
#define NC_SQ_E8 60
#define NC_SQ_F8 61
#define NC_SQ_G8 62
#define NC_SQ_H8 63

/* Directional offsets */
#define NC_SQ_SOUTHWEST -9
#define NC_SQ_SOUTH -8
#define NC_SQ_SOUTHEAST -7
#define NC_SQ_WEST -1
#define NC_SQ_EAST 1
#define NC_SQ_NORTHWEST 7
#define NC_SQ_NORTH 8
#define NC_SQ_NORTHEAST 9

const char* nc_square_tostr(nc_square in);
nc_square nc_square_fromstr(const char* inp);

static inline int nc_square_is_valid(nc_square in) {
	return (in >= 0 && in < 64);
}

static inline nc_square nc_square_at(int r, int f) {
	nc_assertf(r >= 0 && f >= 0 && r < 8 && f < 8, "Invalid coordinates: r%d, f%d", r, f);
	return (r * 8 + f);
}

static inline nc_square nc_square_shift(nc_square in, int dir) {
	nc_assert(nc_square_is_valid(in));
	return in + dir;
}

static inline nc_square nc_square_rank(nc_square in) {
	nc_assert(nc_square_is_valid(in));
	return in >> 3;
}

static inline nc_square nc_square_file(nc_square in) {
	nc_assert(nc_square_is_valid(in));
	return in & 7;
}
