#include "position.h"

#include <string.h>

void nc_position_init(nc_position* dst) {
    memset(dst, 0, sizeof *dst);

    /* Start at 0 ply */
    dst->ply = 0;

    /* Set the board pieces to NULL */
    for (nc_square i = 0; i < 64; ++i) {
        dst->board[i] = NC_PIECE_NULL;
    }

    /* Set initial irreversible state */
    dst->states[dst->ply].ep_target = NC_SQ_NULL;
    dst->states[dst->ply].halfmove_clock = 0;

    /* Sets the initial castle state. */
    nc_position_update_castling(dst, NC_CASTLE_ALL);

    /* Set color to move */
    dst->color_to_move = NC_WHITE;

    /* Place white pieces */
    nc_position_place_piece(dst, NC_WHITE_ROOK, NC_SQ_A1);
    nc_position_place_piece(dst, NC_WHITE_KNIGHT, NC_SQ_B1);
    nc_position_place_piece(dst, NC_WHITE_BISHOP, NC_SQ_C1);
    nc_position_place_piece(dst, NC_WHITE_QUEEN, NC_SQ_D1);
    nc_position_place_piece(dst, NC_WHITE_KING, NC_SQ_E1);
    nc_position_place_piece(dst, NC_WHITE_BISHOP, NC_SQ_F1);
    nc_position_place_piece(dst, NC_WHITE_KNIGHT, NC_SQ_G1);
    nc_position_place_piece(dst, NC_WHITE_ROOK, NC_SQ_H1);

    /* Place black pieces */
    nc_position_place_piece(dst, NC_BLACK_ROOK, NC_SQ_A8);
    nc_position_place_piece(dst, NC_BLACK_KNIGHT, NC_SQ_B8);
    nc_position_place_piece(dst, NC_BLACK_BISHOP, NC_SQ_C8);
    nc_position_place_piece(dst, NC_BLACK_QUEEN, NC_SQ_D8);
    nc_position_place_piece(dst, NC_BLACK_KING, NC_SQ_E8);
    nc_position_place_piece(dst, NC_BLACK_BISHOP, NC_SQ_F8);
    nc_position_place_piece(dst, NC_BLACK_KNIGHT, NC_SQ_G8);
    nc_position_place_piece(dst, NC_BLACK_ROOK, NC_SQ_H8);

    /* Place pawns */
    for (int f = 0; f < 8; ++f) {
        nc_position_place_piece(dst, NC_WHITE_PAWN, nc_square_at(1, f));
        nc_position_place_piece(dst, NC_BLACK_PAWN, nc_square_at(6, f));
    }
}

void nc_position_make_move(nc_position* dst, nc_move move) {
    nc_abort("Not implemented!");
}

void nc_position_unmake_move(nc_position* dst, nc_move move) {
    nc_abort("Not implemented!");
}

void nc_position_legal_moves(nc_position* dst, nc_movelist* out) {
    nc_abort("Not implemented!");
}

void nc_position_update_castling(nc_position* dst, int castling) {
    /* Unset the zkey for castling from the last ply if applicable. */
    if (dst->ply) {
        dst->key ^= nc_zobrist_castle(dst->states[dst->ply - 1].castling);
    }

    dst->states[dst->ply].castling = castling;
    dst->key ^= nc_zobrist_castle(castling);
}

void nc_position_place_piece(nc_position* dst, nc_piece p, nc_square at) {
    nc_assert(dst->board[at] == NC_PIECE_NULL);

    nc_position_flip_piece(dst, p, at);
    dst->board[at] = p;
}

void nc_position_flip_piece(nc_position* dst, nc_piece p, nc_square at) {
    nc_bb tmask = nc_bb_mask(at);

    dst->global ^= tmask;
    dst->color[nc_piece_color(p)] ^= tmask;
    dst->piece[nc_piece_type(p)] ^= tmask;

    dst->key ^= nc_zobrist_piece(at, p);
}

void nc_position_move_piece(nc_position* dst, nc_square from, nc_square to) {
    nc_assert(dst->board[from] != NC_PIECE_NULL);
    nc_assert(dst->board[to] == NC_PIECE_NULL);

    nc_position_flip_piece(dst, dst->board[from], from);
    nc_position_flip_piece(dst, dst->board[from], to);

    dst->board[to] = dst->board[from];
    dst->board[from] = NC_PIECE_NULL;
}

nc_piece nc_position_capture_piece(nc_position* dst, nc_square from, nc_square to) {
    nc_piece psrc = dst->board[from];
    nc_piece pdst = dst->board[to];

    nc_assert(psrc != NC_PIECE_NULL);
    nc_assert(pdst != NC_PIECE_NULL);
    nc_assert(nc_piece_color(psrc) != nc_piece_color(pdst));

    nc_position_flip_piece(dst, psrc, from);
    nc_position_flip_piece(dst, pdst, to);
    nc_position_flip_piece(dst, psrc, to);

    dst->board[to] = psrc;
    dst->board[from] = NC_PIECE_NULL;

    return pdst;
}
