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
    dst->states[dst->ply].halfmove_clock = 0;
    dst->states[dst->ply].captured = NC_PIECE_NULL;
    dst->states[dst->ply].captured_at = NC_SQ_NULL;
    dst->states[dst->ply].lastmove = NC_MOVE_NULL;

    /* Sets the initial castle state. */
    nc_position_update_castling(dst, -1, NC_CASTLE_ALL);

    /* Set the initial EP target. */
    nc_position_update_ep_target(dst, NC_SQ_NULL, NC_SQ_NULL);

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

void nc_position_make_move(nc_position* p, nc_move move) {
    /* Copy state to next ply */
    nc_pstate* last = p->states + p->ply, *next = p->states + ++p->ply;
    *next = *last;

    /* Set lastmove */
    next->lastmove = move;

    nc_square src = nc_move_get_src(move);
    nc_square dst = nc_move_get_dst(move);

    if (move & NC_CASTLE) {
        /* Castle move */
        /* New castle rights mask */
        int newmask = (p->color_to_move == NC_WHITE) ? (NC_BLACK_KINGSIDE | NC_BLACK_QUEENSIDE) : (NC_WHITE_KINGSIDE | NC_WHITE_QUEENSIDE);

        if (dst == NC_SQ_C1) {
            /* white queenside */
            nc_position_move_piece(p, NC_SQ_A1, NC_SQ_D1);
            nc_position_move_piece(p, NC_SQ_E1, NC_SQ_C1);
        } else if (dst == NC_SQ_G1) {
            /* white kingside */
            nc_position_move_piece(p, NC_SQ_H1, NC_SQ_F1);
            nc_position_move_piece(p, NC_SQ_E1, NC_SQ_G1);
        } else if (dst == NC_SQ_C8) {
            /* black queenside */
            nc_position_move_piece(p, NC_SQ_A8, NC_SQ_D8);
            nc_position_move_piece(p, NC_SQ_E8, NC_SQ_C8);
        } else if (dst == NC_SQ_G8) {
            /* black kingside */
            nc_position_move_piece(p, NC_SQ_H8, NC_SQ_F8);
            nc_position_move_piece(p, NC_SQ_E8, NC_SQ_G8);
        }

        next->captured = NC_PIECE_NULL;
        next->captured_at = NC_SQ_NULL;
        next->halfmove_clock++;

        nc_position_update_ep_target(p, last->ep_target, NC_SQ_NULL);
        nc_position_update_castling(p, last->castling, last->castling & newmask);
    } else {
        /* Captures, quiets, promotions */
        if (move & NC_CAPTURE) {
            if (dst == last->ep_target) {
                /* En passant capture, hint where to place the piece in unmake */
                next->captured_at = last->ep_target + ((p->color_to_move == NC_WHITE) ? NC_SQ_SOUTH : NC_SQ_NORTH);
                next->captured = nc_position_remove_piece(p, next->captured_at);
                nc_position_move_piece(p, src, dst);
            } else {
                /* Normal capture, piece should be replaced where it was captured */
                next->captured = nc_position_capture_piece(p, src, dst);
                next->captured_at = dst;
            }

            next->halfmove_clock = 0;
        } else {
            /* Normal move. Unset capture flags */
            nc_position_move_piece(p, src, dst);

            next->halfmove_clock++;
            next->captured = NC_PIECE_NULL;
            next->captured_at = NC_SQ_NULL;
        }

        if (move & NC_PROMOTION) {
            nc_position_replace_piece(p, nc_piece_make(p->color_to_move, nc_move_get_ptype(move)), dst);
        }

        if (move & NC_PAWNJUMP) {
            nc_position_update_ep_target(p, last->ep_target, dst + ((p->color_to_move == NC_WHITE) ? NC_SQ_SOUTH : NC_SQ_NORTH));
        } else {
            nc_position_update_ep_target(p, last->ep_target, NC_SQ_NULL);
        }

        /* Update castle rights depending on src, dst squares */
        int newrights = last->castling;

        if (src == NC_SQ_E1) {
            newrights &= NC_BLACK_QUEENSIDE | NC_BLACK_KINGSIDE;
        } else if (src == NC_SQ_E8) {
            newrights &= NC_WHITE_QUEENSIDE | NC_WHITE_KINGSIDE;
        } else if (src == NC_SQ_A1 || dst == NC_SQ_A1) {
            newrights &= ~NC_WHITE_QUEENSIDE;
        } else if (src == NC_SQ_H1 || dst == NC_SQ_H1) {
            newrights &= ~NC_WHITE_KINGSIDE;
        } else if (src == NC_SQ_A8 || dst == NC_SQ_A8) {
            newrights &= ~NC_BLACK_QUEENSIDE;
        } else if (src == NC_SQ_H8 || dst == NC_SQ_H8) {
            newrights &= ~NC_BLACK_KINGSIDE;
        }

        if (newrights != last->castling) {
            nc_position_update_castling(p, last->castling, newrights);
        }
    }

    p->color_to_move = nc_colorflip(p->color_to_move);
}

void nc_position_unmake_move(nc_position* p, nc_move move) {
    /* Pop back state */
    nc_pstate* cur = p->states + p->ply, *prev = p->states + --p->ply;

    /* Check the move is the right one */
    nc_assert(move == cur->lastmove);

    /* Flip color to move early */
    /* p->color_to_move is the color that made the move we are unmaking. */
    p->color_to_move = nc_colorflip(p->color_to_move);

    nc_square src = nc_move_get_src(move);
    nc_square dst = nc_move_get_dst(move);

    if (move & NC_CASTLE) {
        /* Unmake castle move */

        if (dst == NC_SQ_C1) {
            /* white queenside */
            nc_position_move_piece(p, NC_SQ_D1, NC_SQ_A1);
            nc_position_move_piece(p, NC_SQ_C1, NC_SQ_E1);
        } else if (dst == NC_SQ_G1) {
            /* white kingside */
            nc_position_move_piece(p, NC_SQ_F1, NC_SQ_H1);
            nc_position_move_piece(p, NC_SQ_G1, NC_SQ_E1);
        } else if (dst == NC_SQ_C8) {
            /* black queenside */
            nc_position_move_piece(p, NC_SQ_D8, NC_SQ_A8);
            nc_position_move_piece(p, NC_SQ_C8, NC_SQ_E8);
        } else if (dst == NC_SQ_G8) {
            /* black kingside */
            nc_position_move_piece(p, NC_SQ_F8, NC_SQ_H8);
            nc_position_move_piece(p, NC_SQ_G8, NC_SQ_E8);
        }
    } else {
        /* Captures, quiets, promotions */

        /* First, unmake promotion */
        if (move & NC_PROMOTION) {
            nc_position_remove_piece(p, dst);
            nc_position_place_piece(p, (p->color_to_move == NC_WHITE) ? NC_WHITE_PAWN : NC_BLACK_PAWN, dst);
        }

        if (move & NC_CAPTURE) {
            /* Unmake capture, we have all the information needed */
            nc_position_move_piece(p, dst, src);
            nc_position_place_piece(p, cur->captured, cur->captured_at);
        } else {
            /* Unmake quiet move */
            nc_position_move_piece(p, dst, src);
        }
    }

    /* Reset castling, ep target regardless of move */
    nc_position_update_ep_target(p, cur->ep_target, prev->ep_target);
    nc_position_update_castling(p, cur->castling, prev->castling);
}

void nc_position_legal_moves(nc_position* dst, nc_movelist* out) {
    nc_abort("Not implemented!");
}

void nc_position_update_castling(nc_position* dst, int oldrights, int newrights) {
    /* Assume oldrights=-1 is an initial call. */
    if (oldrights >= 0) {
        dst->key ^= nc_zobrist_castle(oldrights);
    }

    dst->states[dst->ply].castling = newrights;
    dst->key ^= nc_zobrist_castle(newrights);
}

void nc_position_update_ep_target(nc_position* dst, nc_square old, nc_square new) {
    if (old != NC_SQ_NULL) {
        dst->key ^= nc_zobrist_ep(nc_square_file(old));
    }

    if (new != NC_SQ_NULL) {
        dst->key ^= nc_zobrist_ep(nc_square_file(new));
    }

    dst->states[dst->ply].ep_target = new;
}

void nc_position_place_piece(nc_position* dst, nc_piece p, nc_square at) {
    nc_assert(dst->board[at] == NC_PIECE_NULL);

    nc_position_flip_piece(dst, p, at);
    dst->board[at] = p;
}

void nc_position_replace_piece(nc_position* dst, nc_piece p, nc_square at) {
    nc_position_flip_piece(dst, dst->board[at], at);
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

nc_piece nc_position_remove_piece(nc_position* dst, nc_square at) {
    nc_assert(dst->board[at] != NC_PIECE_NULL);

    nc_position_flip_piece(dst, dst->board[at], at);

    nc_piece ret = dst->board[at];
    dst->board[at] = NC_PIECE_NULL;

    return ret;
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

void nc_position_dump(nc_position* p, FILE* out) {
    /* Pretty-print the board and current state */

    char boardbits[8][9];

    int line = 0;
    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            nc_piece c = p->board[nc_square_at(r, f)];
            boardbits[line][f] = (c == NC_PIECE_NULL) ? '.' : nc_piece_touci(c);
        }

        boardbits[line][8] = '\0';
        ++line;
    }

    char castlestr[5] = {0};
    int i = 0, castling = p->states[p->ply].castling;

    if (castling & NC_WHITE_KINGSIDE) {
        castlestr[i++] = 'K';
    }

    if (castling & NC_WHITE_QUEENSIDE) {
        castlestr[i++] = 'Q';
    }

    if (castling & NC_BLACK_KINGSIDE) {
        castlestr[i++] = 'k';
    }

    if (castling & NC_BLACK_QUEENSIDE) {
        castlestr[i++] = 'q';
    }

    char captured = (p->states[p->ply].captured == NC_PIECE_NULL) ? '-' : nc_piece_touci(p->states[p->ply].captured);

    fprintf(out, "   +--------+ ply: %d, ep_target: %s\n", p->ply, nc_square_tostr(p->states[p->ply].ep_target));
    fprintf(out, " 8 |%s| captured: %c, captured_at: %s\n", boardbits[0], captured, nc_square_tostr(p->states[p->ply].captured_at));
    fprintf(out, " 7 |%s| color_to_move: %c, key = %llx\n", boardbits[1], nc_colorchar(p->color_to_move), (unsigned long long) p->key);
    fprintf(out, " 6 |%s| lastmove: %s, castling: %s\n", boardbits[2], nc_move_tostr(p->states[p->ply].lastmove), castlestr);
    fprintf(out, " 5 |%s|\n", boardbits[3]);
    fprintf(out, " 4 |%s|\n", boardbits[4]);
    fprintf(out, " 3 |%s|\n", boardbits[5]);
    fprintf(out, " 2 |%s|\n", boardbits[6]);
    fprintf(out, " 1 |%s|\n", boardbits[7]);
    fprintf(out, "   +--------+\n");
    fprintf(out, "    abcdefgh\n");
}
