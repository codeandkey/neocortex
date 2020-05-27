#pragma once

/*
 * Moves are packed into a single int.
 *
 * LSB
 * 0-5: target square
 * 6-11: source square
 * 12-19: flags
 * 20-23: promotion type
 */

#include "piece.h"
#include "square.h"

#include <string.h>

typedef int nc_move;

/* Move literals */
#define NC_MOVE_NULL -1

/* Move flags */
#define NC_PROMOTION 0x1000
#define NC_CAPTURE   0x2000
#define NC_PAWNJUMP  0x4000
#define NC_CASTLE	0x8000

/* Movelist compile time config. */
#define NC_MOVELIST_LEN 256

typedef struct {
	nc_move moves[NC_MOVELIST_LEN];
	int len;
} nc_movelist;

const char* nc_move_tostr(nc_move in);
nc_move nc_move_fromstr(const char* in);

static inline nc_move nc_move_make(nc_square from, nc_square to) {
	return (from << 6) | to;
} 

static inline nc_move nc_move_promotion(nc_move inp, int ptype) {
	return inp | NC_PROMOTION | (ptype << 20);
}

static inline nc_move nc_move_capture(nc_move inp) {
	return inp | NC_CAPTURE;
}

static inline nc_square nc_move_get_src(nc_move inp) {
	return (inp >> 6) & 0x3F;
}

static inline nc_square nc_move_get_dst(nc_move inp) {
	return inp & 0x3F;
}

static inline int nc_move_get_ptype(nc_move inp) {
	return (inp >> 20) & 0xF;
}

static inline void nc_movelist_push(nc_movelist* out, nc_move m) {
	out->moves[out->len++] = m;
}

static inline void nc_movelist_clear(nc_movelist* out) {
	out->len = 0;
}

static inline void nc_movelist_concat(nc_movelist* dst, nc_movelist* src) {
	memcpy(dst->moves + dst->len, src->moves, src->len * sizeof src->moves[0]);
	dst->len += src->len;
}

nc_move nc_movelist_match(nc_movelist* lst, nc_move in);
