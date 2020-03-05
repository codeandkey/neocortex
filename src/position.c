#include "position.h"
#include "magic.h"
#include "basic.h"
#include "eval_consts.h"

#include <string.h>

#ifdef NC_DEBUG
#define nc_passertf(p, c, x, ...) if (!(c)) { nc_position_dump(p, stderr, 0); nc_assertf(0, x, ##__VA_ARGS__); }
#else
#define nc_passertf(p, c, x, ...)
#endif

void nc_position_init(nc_position* dst) {
    memset(dst, 0, sizeof *dst);

    /* Start at neutral eval. */
    nc_pst_zero(&dst->pst);

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
    dst->states[dst->ply].check = 0;

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

void nc_position_init_fen(nc_position* dst, const char* fen) {
    if (!strcmp(fen, "startpos")) {
        nc_position_init(dst);
        return;
    }

    memset(dst, 0, sizeof *dst);

    /* Copy FEN for parsing */
    int fen_len = strlen(fen);
    char* fen_copy = malloc(fen_len + 1);
    memcpy(fen_copy, fen, fen_len + 1);

    /* Grab FEN fields */
    char* fen_board = strtok(fen_copy, " \n");
    char* fen_ctm = strtok(NULL, " \n");
    char* fen_castling = strtok(NULL, " \n");
    char* fen_ep_target = strtok(NULL, " \n");
    char* fen_halfmove_clock = strtok(NULL, " \n");
    char* fen_fullmove_number = strtok(NULL, " \n");

    if (!fen_board || !fen_ctm || !fen_castling || !fen_ep_target || !fen_halfmove_clock || !fen_fullmove_number) {
        nc_abort("Invalid FEN: missing fields!");
    }

    /* Set ply and other unknown state */
    dst->ply = 0;
    dst->states[dst->ply].captured = NC_PIECE_NULL;
    dst->states[dst->ply].captured_at = NC_SQ_NULL;
    dst->states[dst->ply].lastmove = NC_MOVE_NULL;

    /* Initialize board memory to null */
    for (int sq = 0; sq < 64; ++sq) {
        dst->board[sq] = NC_PIECE_NULL;
    }

    /* Parse board pieces */
    int rind = 7;
    for (char* rank = strtok(fen_board, "/"); rank; rank = strtok(NULL, "/"), --rind) {
        int len = strlen(rank), file = 0;
        for (int i = 0; i < len; ++i) {
            if (rank[i] >= '1' && rank[i] <= '8') {
                /* skip pieces, set board to null */
                int count = rank[i] - '0';
                for (int k = 0; k < count; ++k) {
                    dst->board[nc_square_at(rind, file + k)] = NC_PIECE_NULL;
                }

                file += count;
            } else {
                nc_position_place_piece(dst, nc_piece_fromuci(rank[i]), nc_square_at(rind, file));
                ++file;
            }
        }
    }

    /* Set color to move */
    switch (*fen_ctm) {
        case 'w':
            dst->color_to_move = NC_WHITE;
            break;
        case 'b':
            dst->color_to_move = NC_BLACK;
            break;
        default:
            nc_abort("Invalid FEN: bad color to move '%s'", fen_ctm);
    }

    /* Set castling state */
    int new_castling = 0;

    for (char* c = fen_castling; *c; ++c) {
        switch (*c) {
            case 'K':
                new_castling |= NC_WHITE_KINGSIDE;
                break;
            case 'Q':
                new_castling |= NC_WHITE_QUEENSIDE;
                break;
            case 'k':
                new_castling |= NC_BLACK_KINGSIDE;
                break;
            case 'q':
                new_castling |= NC_BLACK_QUEENSIDE;
                break;
            case '-':
                break;
            default:
                nc_abort("Invalid FEN: invalid char '%c' in castling state", *c);
        }
    }

    nc_position_update_castling(dst, -1, new_castling);

    /* Set EP target */
    nc_square ep = nc_square_fromstr(fen_ep_target);
    nc_position_update_ep_target(dst, NC_SQ_NULL, ep);

    /* Set halfmove clock */
    dst->states[dst->ply].halfmove_clock = strtol(fen_halfmove_clock, NULL, 10);

    /* Ignore the full move number for now */

    /* Update check state */
    nc_bb kingmask = dst->piece[NC_KING] & dst->color[dst->color_to_move];
    dst->states[dst->ply].check = (nc_position_gen_attack_mask_for(dst, nc_colorflip(dst->color_to_move), kingmask) & kingmask) != 0ULL;

    free(fen_copy);
}

void nc_position_make_move(nc_position* p, nc_move move) {
    /* Put key in history */
    p->states[p->ply].key = p->key;

    /* Copy state to next ply */
    nc_pstate* last = p->states + p->ply, *next = p->states + ++p->ply;
    *next = *last;

    /* Flip BTM key */
    p->key ^= nc_zobrist_black_to_move();

    /* Set lastmove */
    next->lastmove = move;

    /* Set check */
    next->check = move & NC_CHECK;

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
        }
        
        if (src == NC_SQ_A1 || dst == NC_SQ_A1) {
            newrights &= ~NC_WHITE_QUEENSIDE;
        }

        if (src == NC_SQ_H1 || dst == NC_SQ_H1) {
            newrights &= ~NC_WHITE_KINGSIDE;
        }

        if (src == NC_SQ_A8 || dst == NC_SQ_A8) {
            newrights &= ~NC_BLACK_QUEENSIDE;
        }

        if (src == NC_SQ_H8 || dst == NC_SQ_H8) {
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
    nc_passertf(p, move == cur->lastmove, "Expected move %d, got move %d", cur->lastmove, move);

    /* Flip BTM key */
    p->key ^= nc_zobrist_black_to_move();

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
    nc_passertf(dst, dst->board[at] == NC_PIECE_NULL, "Destination piece not null! (%c)", nc_piece_touci(dst->board[at]));

    nc_position_flip_piece(dst, p, at);
    nc_pst_add(&dst->pst, p, at);

    dst->board[at] = p;
}

void nc_position_replace_piece(nc_position* dst, nc_piece p, nc_square at) {
    nc_passertf(dst, dst->board[at] != NC_PIECE_NULL, "Destination piece null! (%c)", nc_piece_touci(dst->board[at]));

    nc_pst_remove(&dst->pst, dst->board[at], at);
    nc_pst_add(&dst->pst, p, at);

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
    nc_passertf(dst, dst->board[from] != NC_PIECE_NULL, "Source piece null! (%c), from=%d, to=%d", nc_piece_touci(dst->board[from]), from, to);
    nc_passertf(dst, dst->board[to] == NC_PIECE_NULL, "Destination piece not null! (%c) from=%d, to=%d", nc_piece_touci(dst->board[to]), from, to);

    nc_position_flip_piece(dst, dst->board[from], from);
    nc_position_flip_piece(dst, dst->board[from], to);

    nc_pst_remove(&dst->pst, dst->board[from], from);
    nc_pst_add(&dst->pst, dst->board[from], to);

    dst->board[to] = dst->board[from];
    dst->board[from] = NC_PIECE_NULL;
}

nc_piece nc_position_remove_piece(nc_position* dst, nc_square at) {
    nc_assert(dst->board[at] != NC_PIECE_NULL);

    nc_position_flip_piece(dst, dst->board[at], at);
    nc_pst_remove(&dst->pst, dst->board[at], at);

    nc_piece ret = dst->board[at];
    dst->board[at] = NC_PIECE_NULL;

    return ret;
}

nc_piece nc_position_capture_piece(nc_position* dst, nc_square from, nc_square to) {
    nc_piece psrc = dst->board[from];
    nc_piece pdst = dst->board[to];

    nc_passertf(dst, psrc != NC_PIECE_NULL, "Source piece is null! from=%d to=%d", from, to);
    nc_passertf(dst, pdst != NC_PIECE_NULL, "Destination piece is null! from=%d to=%d", from, to);
    nc_passertf(dst, nc_piece_color(psrc) != nc_piece_color(pdst), "Source color matches dest color!");

    nc_position_flip_piece(dst, psrc, from);
    nc_position_flip_piece(dst, pdst, to);
    nc_position_flip_piece(dst, psrc, to);

    nc_pst_remove(&dst->pst, pdst, to);
    nc_pst_remove(&dst->pst, psrc, from);
    nc_pst_add(&dst->pst, psrc, to);

    dst->board[to] = psrc;
    dst->board[from] = NC_PIECE_NULL;

    return pdst;
}

void nc_position_dump(nc_position* p, FILE* out, int include_moves) {
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
    fprintf(out, " 5 |%s| global: %llx\n", boardbits[3], (unsigned long long) p->global);
    fprintf(out, " 4 |%s| check: %d\n", boardbits[4], p->states[p->ply].check != 0);
    fprintf(out, " 3 |%s|\n", boardbits[5]);
    fprintf(out, " 2 |%s|\n", boardbits[6]);
    fprintf(out, " 1 |%s|\n", boardbits[7]);
    fprintf(out, "   +--------+\n");
    fprintf(out, "    abcdefgh\n");

    fprintf(out, "^ game history: ");

    for (int i = 1; i <= p->ply; ++i) {
        fprintf(out, "%s ", nc_move_tostr(p->states[i].lastmove));
    }

    fprintf(out, "\n");

    if (include_moves) {
        nc_movelist moves;
        nc_movelist_clear(&moves);
        nc_position_legal_moves(p, &moves);

        fprintf(out, "^ legal moves (%d): ", moves.len);

        for (int i = 0; i < moves.len; ++i) {
            fprintf(out, "%s ", nc_move_tostr(moves.moves[i]));
        }

        fprintf(out, "\n");
    }

    fprintf(out, "^ opposite_attacks:\n%s", nc_bb_tostr(nc_position_gen_attack_mask_for(p, nc_colorflip(p->color_to_move), 0)));
}

nc_eval nc_position_score(nc_position* dst, nc_movelist* out) {
    nc_movelist_clear(out);
    nc_position_legal_moves(dst, out);

    /* Check for terminal nodes here. */
    if (!out->len) {
        if (dst->states[dst->ply].check) return NC_EVAL_MIN;
        return NC_EVAL_CONTEMPT;
    }

    nc_eval thin_score = nc_position_score_thin(dst);
    nc_eval mobility_bonus = NC_EVAL_MOBILITY * out->len;

    return thin_score + mobility_bonus;
}

nc_eval nc_position_score_thin(nc_position* dst) {
    float phase = nc_position_phase(dst);

    nc_eval pawn_structure_bonus = NC_EVAL_PAWN_STRUCTURE * (nc_position_pawn_structure(dst, dst->color_to_move) - nc_position_pawn_structure(dst, nc_colorflip(dst->color_to_move)));

    return nc_pst_get_score(&dst->pst, dst->color_to_move, phase) + pawn_structure_bonus;
}

float nc_position_phase(nc_position* dst) {
    return nc_pst_get_phase(&dst->pst);
}

int nc_position_pawn_structure(nc_position* dst, nc_color col) {
    nc_bb pawns = dst->piece[NC_PAWN] & dst->color[col];

    nc_bb pawns_withright = pawns & ~NC_BB_FILEH;
    nc_bb pawns_withleft = pawns & ~NC_BB_FILEA;

    nc_bb attacked_mask = 0;

    if (col == NC_WHITE) {
        attacked_mask |= nc_bb_shift(pawns_withright, NC_SQ_NORTHEAST);
        attacked_mask |= nc_bb_shift(pawns_withleft, NC_SQ_NORTHWEST);
    } else {
        attacked_mask |= nc_bb_shift(pawns_withright, NC_SQ_SOUTHEAST);
        attacked_mask |= nc_bb_shift(pawns_withleft, NC_SQ_SOUTHWEST);
    }

    return __builtin_popcountll(attacked_mask & pawns);
}

void nc_position_legal_moves(nc_position* dst, nc_movelist* out) {
    nc_movelist pseudolegal;
    nc_movelist_clear(&pseudolegal);

    if (!dst->states[dst->ply].check) nc_position_gen_castle_moves(dst, &pseudolegal);

    nc_position_gen_pawn_moves(dst, &pseudolegal, 1);
    nc_position_gen_queen_moves(dst, &pseudolegal, 1);
    nc_position_gen_rook_moves(dst, &pseudolegal, 1);
    nc_position_gen_knight_moves(dst, &pseudolegal, 1);
    nc_position_gen_bishop_moves(dst, &pseudolegal, 1);
    nc_position_gen_king_moves(dst, &pseudolegal, 1);

    nc_position_gen_pawn_moves(dst, &pseudolegal, 0);
    nc_position_gen_queen_moves(dst, &pseudolegal, 0);
    nc_position_gen_rook_moves(dst, &pseudolegal, 0);
    nc_position_gen_knight_moves(dst, &pseudolegal, 0);
    nc_position_gen_bishop_moves(dst, &pseudolegal, 0);
    nc_position_gen_king_moves(dst, &pseudolegal, 0);

    /* To test if a move is legal, we have to apply it and then test for illegal check. */
    for (int i = 0; i < pseudolegal.len; ++i) {
        nc_move cur = pseudolegal.moves[i];

        /* Here we also have to perform castle check testing. */
        nc_bb badmask = 0;

        nc_position_make_move(dst, cur);

        if (cur & NC_CASTLE) {
            nc_square mvdst = nc_move_get_dst(cur);

            switch (mvdst) {
            case NC_SQ_C1:
                badmask = nc_bb_mask(NC_SQ_E1) | nc_bb_mask(NC_SQ_D1) | nc_bb_mask(NC_SQ_C1);
                break;
            case NC_SQ_G1:
                badmask = nc_bb_mask(NC_SQ_E1) | nc_bb_mask(NC_SQ_F1) | nc_bb_mask(NC_SQ_G1);
                break;
            case NC_SQ_C8:
                badmask = nc_bb_mask(NC_SQ_E8) | nc_bb_mask(NC_SQ_D8) | nc_bb_mask(NC_SQ_C8);
                break;
            case NC_SQ_G8:
                badmask = nc_bb_mask(NC_SQ_E8) | nc_bb_mask(NC_SQ_F8) | nc_bb_mask(NC_SQ_G8);
                break;
            }
        } else {
            badmask = dst->piece[NC_KING] & dst->color[nc_colorflip(dst->color_to_move)];
        }

        /* Check if the move is legal here, and also check if the move delivers check. */
        /* If the move is illegal, it is tossed. */
        /* If the move delivers check, the NC_CHECK flag is added to the move. */

        nc_bb other_att = nc_position_gen_attack_mask_for(dst, dst->color_to_move, badmask);

        if (!(other_att & badmask)) {
            /* Move is legal. Test if it delivers check. */

            nc_bb other_king = dst->piece[NC_KING] & dst->color[dst->color_to_move];
            nc_bb att = nc_position_gen_attack_mask_for(dst, nc_colorflip(dst->color_to_move), other_king);

            if (att & other_king) {
                nc_movelist_push(out, cur | NC_CHECK);
            } else {
                nc_movelist_push(out, cur);
            }
        }

        nc_position_unmake_move(dst, cur);
    }
}

int nc_position_is_repetition(nc_position* p) {
    /* walk through the ply and see if we've seen the key before */
    for (int i = p->ply - 1; i >= 0; --i) {
        if (p->states[i].key == p->key) return 1;
    }

    return 0;
}

void nc_position_gen_pawn_moves(nc_position* p, nc_movelist* out, int captures) {
    nc_bb all_pawns = p->piece[NC_PAWN] & p->color[p->color_to_move];
    nc_bb promotion_mask = (p->color_to_move == NC_WHITE) ? NC_BB_RANK7 : NC_BB_RANK2;

    nc_bb promoting_pawns = all_pawns & promotion_mask;
    nc_bb pawns = all_pawns & ~promotion_mask;

    if (captures) {
        /* Add pawn captures */

        nc_bb capture_mask = p->color[nc_colorflip(p->color_to_move)];

        /* Add ep target to potential captures */
        if (p->states[p->ply].ep_target) {
            capture_mask |= nc_bb_mask(p->states[p->ply].ep_target);
        }

        int dir_right = (p->color_to_move == NC_WHITE) ? NC_SQ_NORTHEAST : NC_SQ_SOUTHEAST;
        int dir_left = (p->color_to_move == NC_WHITE) ? NC_SQ_NORTHWEST : NC_SQ_SOUTHWEST;

        /* Add nonpromoting captures */
        if (pawns) {
            nc_bb right_attacks = nc_bb_shift(pawns & ~NC_BB_FILEH, dir_right) & capture_mask;

            while (right_attacks) {
                nc_square dst = nc_bb_poplsb(&right_attacks);
                nc_movelist_push(out, nc_move_make(dst - dir_right, dst) | NC_CAPTURE);
            }

            nc_bb left_attacks = nc_bb_shift(pawns & ~NC_BB_FILEA, dir_left) & capture_mask;

            while (left_attacks) {
                nc_square dst = nc_bb_poplsb(&left_attacks);
                nc_movelist_push(out, nc_move_make(dst - dir_left, dst) | NC_CAPTURE);
            }
        }

        /* Add promoting captures */
        if (promoting_pawns) {
            nc_bb right_attacks = nc_bb_shift(promoting_pawns & ~NC_BB_FILEH, dir_right) & capture_mask;

            while (right_attacks) {
                nc_square dst = nc_bb_poplsb(&right_attacks);
                nc_move template = nc_move_make(dst - dir_right, dst) | NC_CAPTURE;
                nc_movelist_push(out, nc_move_promotion(template, NC_QUEEN));
                nc_movelist_push(out, nc_move_promotion(template, NC_KNIGHT));
                nc_movelist_push(out, nc_move_promotion(template, NC_ROOK));
                nc_movelist_push(out, nc_move_promotion(template, NC_BISHOP));
            }

            nc_bb left_attacks = nc_bb_shift(promoting_pawns & ~NC_BB_FILEA, dir_left) & capture_mask;

            while (left_attacks) {
                nc_square dst = nc_bb_poplsb(&left_attacks);
                nc_move template = nc_move_make(dst - dir_left, dst) | NC_CAPTURE;
                nc_movelist_push(out, nc_move_promotion(template, NC_QUEEN));
                nc_movelist_push(out, nc_move_promotion(template, NC_KNIGHT));
                nc_movelist_push(out, nc_move_promotion(template, NC_ROOK));
                nc_movelist_push(out, nc_move_promotion(template, NC_BISHOP));
            }
        }
    } else {
        /* Add pawn advance moves */
        nc_bb starting_rank = (p->color_to_move == NC_WHITE) ? NC_BB_RANK2 : NC_BB_RANK7;
        int adv_dir = (p->color_to_move == NC_WHITE) ? NC_SQ_NORTH : NC_SQ_SOUTH;
        nc_bb starting_pawns = pawns & starting_rank;

        /* Add promoting pawn advances */
        if (promoting_pawns) {
            nc_bb singles = nc_bb_shift(promoting_pawns, adv_dir) & ~p->global;

            while (singles) {
                nc_square dst = nc_bb_poplsb(&singles);
                nc_move template = nc_move_make(dst - adv_dir, dst);
                nc_movelist_push(out, nc_move_promotion(template, NC_QUEEN));
                nc_movelist_push(out, nc_move_promotion(template, NC_KNIGHT));
                nc_movelist_push(out, nc_move_promotion(template, NC_ROOK));
                nc_movelist_push(out, nc_move_promotion(template, NC_BISHOP));
            }
        }

        /* Add pawn jump moves */
        if (starting_pawns) {
            nc_bb singles = nc_bb_shift(starting_pawns, adv_dir) & ~p->global;
            nc_bb doubles = nc_bb_shift(singles, adv_dir) & ~p->global;

            while (doubles) {
                nc_square dst = nc_bb_poplsb(&doubles);
                nc_movelist_push(out, nc_move_make(dst - 2 * adv_dir, dst) | NC_PAWNJUMP);
            }
        }

        /* Add nonpromoting pawn single advances */
        if (pawns) {
            nc_bb singles = nc_bb_shift(pawns, adv_dir) & ~p->global;

            while (singles) {
                nc_square dst = nc_bb_poplsb(&singles);
                nc_movelist_push(out, nc_move_make(dst - adv_dir, dst));
            }
        }
    }
}

void nc_position_gen_rook_moves(nc_position* p, nc_movelist* out, int captures) {
    nc_bb rooks = p->piece[NC_ROOK] & p->color[p->color_to_move];

    if (captures) {
        while (rooks) {
            nc_square src = nc_bb_poplsb(&rooks);
            nc_bb captures = nc_magic_query_rook_attacks(src, p->global) & p->color[nc_colorflip(p->color_to_move)];

            while (captures) {
                nc_square dst = nc_bb_poplsb(&captures);
                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }
        }
    } else {
        while (rooks) {
            nc_square src = nc_bb_poplsb(&rooks);
            nc_bb moves = nc_magic_query_rook_attacks(src, p->global) & ~p->global;

            while (moves) {
                nc_square dst = nc_bb_poplsb(&moves);
                nc_movelist_push(out, nc_move_make(src, dst));
            }
        }
    }
}

void nc_position_gen_knight_moves(nc_position* p, nc_movelist* out, int captures) {
    nc_bb knights = p->piece[NC_KNIGHT] & p->color[p->color_to_move];

    if (captures) {
        while (knights) {
            nc_square src = nc_bb_poplsb(&knights);
            nc_bb captures = nc_basic_knight_attacks(src) & p->color[nc_colorflip(p->color_to_move)];

            while (captures) {
                nc_square dst = nc_bb_poplsb(&captures);
                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }
        }
    } else {
        while (knights) {
            nc_square src = nc_bb_poplsb(&knights);
            nc_bb moves = nc_basic_knight_attacks(src) & ~p->global;

            while (moves) {
                nc_square dst = nc_bb_poplsb(&moves);
                nc_movelist_push(out, nc_move_make(src, dst));
            }
        }
    }
}

void nc_position_gen_bishop_moves(nc_position* p, nc_movelist* out, int captures) {
    nc_bb bishops = p->piece[NC_BISHOP] & p->color[p->color_to_move];

    if (captures) {
        while (bishops) {
            nc_square src = nc_bb_poplsb(&bishops);
            nc_bb captures = nc_magic_query_bishop_attacks(src, p->global) & p->color[nc_colorflip(p->color_to_move)];

            while (captures) {
                nc_square dst = nc_bb_poplsb(&captures);
                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }
        }
    } else {
        while (bishops) {
            nc_square src = nc_bb_poplsb(&bishops);
            nc_bb moves = nc_magic_query_bishop_attacks(src, p->global) & ~p->global;

            while (moves) {
                nc_square dst = nc_bb_poplsb(&moves);
                nc_movelist_push(out, nc_move_make(src, dst));
            }
        }
    }
}

void nc_position_gen_queen_moves(nc_position* p, nc_movelist* out, int captures) {
    nc_bb queens = p->piece[NC_QUEEN] & p->color[p->color_to_move];

    if (captures) {
        while (queens) {
            nc_square src = nc_bb_poplsb(&queens);
            nc_bb mask = nc_magic_query_rook_attacks(src, p->global) | nc_magic_query_bishop_attacks(src, p->global);
            nc_bb captures = mask & p->color[nc_colorflip(p->color_to_move)];

            while (captures) {
                nc_square dst = nc_bb_poplsb(&captures);
                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }
        }
    } else {
        while (queens) {
            nc_square src = nc_bb_poplsb(&queens);
            nc_bb mask = nc_magic_query_rook_attacks(src, p->global) | nc_magic_query_bishop_attacks(src, p->global);
            nc_bb moves = mask & ~p->global;

            while (moves) {
                nc_square dst = nc_bb_poplsb(&moves);
                nc_movelist_push(out, nc_move_make(src, dst));
            }
        }
    }
}

void nc_position_gen_king_moves(nc_position* p, nc_movelist* out, int captures) {
    nc_bb kings = p->piece[NC_KING] & p->color[p->color_to_move];

    if (captures) {
        while (kings) {
            nc_square src = nc_bb_poplsb(&kings);
            nc_bb captures = nc_basic_king_attacks(src) & p->color[nc_colorflip(p->color_to_move)];

            while (captures) {
                nc_square dst = nc_bb_poplsb(&captures);
                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }
        }
    } else {
        while (kings) {
            nc_square src = nc_bb_poplsb(&kings);
            nc_bb moves = nc_basic_king_attacks(src) & ~p->global;

            while (moves) {
                nc_square dst = nc_bb_poplsb(&moves);
                nc_movelist_push(out, nc_move_make(src, dst));
            }
        }
    }
}

void nc_position_gen_castle_moves(nc_position* dst, nc_movelist* out) {
    if (dst->color_to_move == NC_WHITE) {
        /* check white castles */
        if (dst->states[dst->ply].castling & NC_WHITE_QUEENSIDE) {
            if (!(dst->global & (nc_bb_mask(NC_SQ_B1) | nc_bb_mask(NC_SQ_C1) | nc_bb_mask(NC_SQ_D1)))) {
                nc_movelist_push(out, nc_move_make(NC_SQ_E1, NC_SQ_C1) | NC_CASTLE);
            }
        }

        if (dst->states[dst->ply].castling & NC_WHITE_KINGSIDE) {
            if (!(dst->global & (nc_bb_mask(NC_SQ_F1) | nc_bb_mask(NC_SQ_G1)))) {
                nc_movelist_push(out, nc_move_make(NC_SQ_E1, NC_SQ_G1) | NC_CASTLE);
            }
        }
    } else {
        /* check black castles */
        if (dst->states[dst->ply].castling & NC_BLACK_QUEENSIDE) {
            if (!(dst->global & (nc_bb_mask(NC_SQ_B8) | nc_bb_mask(NC_SQ_C8) | nc_bb_mask(NC_SQ_D8)))) {
                nc_movelist_push(out, nc_move_make(NC_SQ_E8, NC_SQ_C8) | NC_CASTLE);
            }
        }

        if (dst->states[dst->ply].castling & NC_BLACK_KINGSIDE) {
            if (!(dst->global & (nc_bb_mask(NC_SQ_F8) | nc_bb_mask(NC_SQ_G8)))) {
                nc_movelist_push(out, nc_move_make(NC_SQ_E8, NC_SQ_G8) | NC_CASTLE);
            }
        }
    }
}

nc_bb nc_position_gen_attack_mask_for(nc_position* dst, nc_color by, nc_bb mask) {
    nc_bb attacked_mask = 0;

    /* Try pieces with the greatest attack span first to try and fail early. */

    /* Test pawn attacks conditionally */
    nc_bb pawns = dst->piece[NC_PAWN] & dst->color[by];
    nc_bb pawns_withright = pawns & ~NC_BB_FILEH;
    nc_bb pawns_withleft = pawns & ~NC_BB_FILEA;

    if (by == NC_WHITE) {
        attacked_mask |= nc_bb_shift(pawns_withright, NC_SQ_NORTHEAST);
        attacked_mask |= nc_bb_shift(pawns_withleft, NC_SQ_NORTHWEST);
    } else {
        attacked_mask |= nc_bb_shift(pawns_withright, NC_SQ_SOUTHEAST);
        attacked_mask |= nc_bb_shift(pawns_withleft, NC_SQ_SOUTHWEST);
    }

    if (attacked_mask & mask) return attacked_mask;

    /* Test queen attacks */
    nc_bb queens = dst->piece[NC_QUEEN] & dst->color[by];

    while (queens) {
        nc_square src = nc_bb_poplsb(&queens);
        attacked_mask |= nc_magic_query_rook_attacks(src, dst->global);
        attacked_mask |= nc_magic_query_bishop_attacks(src, dst->global);
        if (attacked_mask & mask) return attacked_mask;
    }

    /* Test rook attacks */
    nc_bb rooks = dst->piece[NC_ROOK] & dst->color[by];

    while (rooks) {
        nc_square src = nc_bb_poplsb(&rooks);
        attacked_mask |= nc_magic_query_rook_attacks(src, dst->global);
        if (attacked_mask & mask) return attacked_mask;
    }

    /* Test bishop attacks */
    nc_bb bishops = dst->piece[NC_BISHOP] & dst->color[by];

    while (bishops) {
        nc_square src = nc_bb_poplsb(&bishops);
        attacked_mask |= nc_magic_query_bishop_attacks(src, dst->global);
        if (attacked_mask & mask) return attacked_mask;
    }

    /* Test knight attacks */
    nc_bb knights = dst->piece[NC_KNIGHT] & dst->color[by];

    while (knights) {
        nc_square src = nc_bb_poplsb(&knights);
        attacked_mask |= nc_basic_knight_attacks(src);
        if (attacked_mask & mask) return attacked_mask;
    }

    /* Test king attacks */
    nc_bb kings = dst->piece[NC_KING] & dst->color[by];

    while (kings) {
        nc_square src = nc_bb_poplsb(&kings);
        attacked_mask |= nc_basic_king_attacks(src);
        if (attacked_mask & mask) return attacked_mask;
    }

    return attacked_mask;
}
