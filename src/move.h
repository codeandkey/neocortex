#pragma once

/*
 * Moves are packed into a single int.
 *
 * LSB
 * 0-5: target square
 * 6-11: source square
 * 12-15: flags
 * 16-19: captured piece type
 * 20-23: promotion type
 */

#include "piece.h"
#include "square.h"

typedef int nc_move_t;

/* Move literals */
#define NC_MOVE_NULL -1

/* Move flags */
#define NC_PROMOTION 0x1000
#define NC_CAPTURE   0x2000
#define NC_PAWNJUMP  0x4000

/* Movelist compile time config. */
#define NC_MOVELIST_LEN 256

typedef struct {
    nc_move_t moves[NC_MOVELIST_LEN];
    int len;
} nc_movelist;

const char* nc_move_tostr(nc_move_t in);
nc_move_t nc_move_fromstr(const char* in);

static inline nc_move_t nc_move_make(nc_square_t from, nc_square_t to) {
    return (from << 6) | to;
} 

static inline nc_move_t nc_move_promotion(nc_move_t inp, int ptype) {
    return inp | NC_PROMOTION | (ptype << 20);
}

static inline nc_move_t nc_move_capture(nc_move_t inp, int ctype) {
    return inp | NC_CAPTURE | (ctype << 16);
}

static inline nc_square_t nc_move_get_src(nc_move_t inp) {
    return (inp >> 6) & 0x3F;
}

static inline nc_square_t nc_move_get_dst(nc_move_t inp) {
    return inp & 0x3F;
}

static inline int nc_move_get_ptype(nc_move_t inp) {
    return (inp >> 20) & 0xF;
}

static inline int nc_move_get_ctype(nc_move_t inp) {
    return (inp >> 16) & 0xF;
}

static inline void nc_movelist_push(nc_movelist* out, nc_move_t m) {
    out->moves[out->len++] = m;
}

static inline void nc_movelist_clear(nc_movelist* out) {
    out->len = 0;
}

nc_move_t nc_movelist_match(nc_movelist* lst, nc_move_t in);
