#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char _nc3_type_chars[7] = {
    '-', 'p', 'b', 'r', 'n', 'q', 'k',
};

static char _nc3_move_strbuf[6];
static char _nc3_square_strbuf[3];
static char _nc3_bitboard_strbuf[73];

const char* nc3_square_tostr(nc3_square sq) {
    if (sq < 0 || sq > 63) return "-";

    _nc3_square_strbuf[0] = NC3_FILE(sq) + 'a';
    _nc3_square_strbuf[1] = NC3_RANK(sq) + '1';

    return (const char*) _nc3_square_strbuf;
}

nc3_square nc3_square_fromstr(const char* inp) {
    int rank = inp[0] - '1', file = inp[1] - 'a';

    if (rank < 0 || rank > 7 || file < 0 || file > 7) return NC3_SQNULL;
    return NC3_SQAT(rank, file);
}

const char* nc3_move_tostr(nc3_move mv) {
    if (mv == NC3_MVNULL) {
        return "0000";
    }

    memset(_nc3_move_strbuf, 0, sizeof _nc3_move_strbuf);
    memcpy(_nc3_move_strbuf, nc3_square_tostr(NC3_GETSRC(mv)), 2);
    memcpy(_nc3_move_strbuf + 2, nc3_square_tostr(NC3_GETDST(mv)), 2);

    if (mv & NC3_PROMOTE) {
        _nc3_move_strbuf[4] = _nc3_type_chars[NC3_GETPTYPE(mv)];
    }

    return (const char*) _nc3_move_strbuf;
}

nc3_move nc3_move_fromstr(const char* inp) {
    nc3_square src = nc3_square_fromstr(inp);
    if (src == NC3_SQNULL) return NC3_MVNULL;

    nc3_square dst = nc3_square_fromstr(inp + 2);
    if (dst == NC3_SQNULL) return NC3_MVNULL;

    int ptype = 0;

    if (inp[4]) {
        switch (inp[4]) {
        case 'r':
            ptype = NC3_ROOK;
            break;
        case 'n':
            ptype = NC3_KNIGHT;
            break;
        case 'b':
            ptype = NC3_BISHOP;
            break;
        case 'q':
            ptype = NC3_QUEEN;
            break;
        default:
            return NC3_MVNULL;
        }
    }

    return NC3_PMOVE(src, dst, ptype);
}

void nc3_movelist_init(nc3_movelist* dst, int count) {
    if (dst->count) {
        dst->moves = malloc(count * sizeof dst->moves[0]);
    } else {
        dst->moves = NULL;
    }

    dst->count = count;
}

void nc3_movelist_free(nc3_movelist* dst) {
    free(dst->moves);
}

void nc3_movelist_add(nc3_movelist* dst, nc3_move move) {
    dst->moves = realloc(dst->moves, ++dst->count * sizeof dst->moves[0]);
    dst->moves[dst->count - 1] = move;
}

const char* nc3_bitboard_tostr(u64 bb) {
    int ind = 0;

    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            _nc3_bitboard_strbuf[ind++] = ((bb >> NC3_SQAT(r, f)) & 1) ? '#' : '.';
        }

        _nc3_bitboard_strbuf[ind++] = '\n';
    }

    return _nc3_bitboard_strbuf;
}
