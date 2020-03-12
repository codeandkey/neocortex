#include "movegen.h"

#include "magic.h"
#include "basic.h"
#include "tt.h"

static int _nc_movegen_pval[] = {
    NC_PAWN,
    NC_KNIGHT,
    NC_BISHOP,
    NC_ROOK,
    NC_QUEEN,
    NC_KING,
};

static void _nc_movegen_gen_castles(nc_position* pos, nc_movelist* out);
static void _nc_movegen_gen_quiets(nc_position* pos, int type, nc_movelist* out);
static void _nc_movegen_gen_captures(nc_position* pos, int victim, int attacker, nc_movelist* out);

void nc_movegen_start_gen(nc_position* pos, nc_movegen* state) {
    state->stage = NC_MOVEGEN_STAGE_BEGIN;
    state->ind = 0;
    state->capture_victim = 4; /* no point in starting with king captures as they are illegal */
    state->capture_attacker = 0;
    nc_movelist_clear(&state->moves);
}

int nc_movegen_next_move(nc_position* pos, nc_movegen* state, nc_move* out) {
    if (state->ind < state->moves.len) {
        /* There are remaining moves from the current stage. Continue yielding until none left. */
        *out = state->moves.moves[state->ind++];
        return 1;
    } else {
        /* No moves left. Advance the current stage or move to the next. */

        switch (state->stage) {
            case NC_MOVEGEN_STAGE_BEGIN:
                ++state->stage;
                return nc_movegen_next_move(pos, state, out);
            case NC_MOVEGEN_STAGE_PV_MOVE:
            {
                /* Try and look up a PV move. */
                nc_ttentry* ent = nc_tt_lookup(pos->key);

                if (ent->key == pos->key && ent->depth) {
                    nc_movelist_push(&state->moves, ent->bestmove);
                }

                ++state->stage;
                return nc_movegen_next_move(pos, state, out);
            }
            case NC_MOVEGEN_STAGE_CAPTURES:
                /* Generate capture moves and advance MVV-LVA counter. */
                if (state->capture_attacker > 5) {
                    state->capture_attacker = 0;

                    /* Move to the next most valuable victim. */
                    if (--state->capture_victim < 0) {
                        /* No more potential victims. Advance to the next stage. */
                        ++state->stage;
                        return nc_movegen_next_move(pos, state, out);
                    }
                }

                /* There are potential captures remaining -- test for potential moves between types. */
                _nc_movegen_gen_captures(pos, _nc_movegen_pval[state->capture_victim], _nc_movegen_pval[state->capture_attacker], &state->moves);

                /* Move to the next least valuable attacker. */
                ++state->capture_attacker;

                return nc_movegen_next_move(pos, state, out);
            case NC_MOVEGEN_STAGE_CASTLES:
                /* Generate castling moves and move to the next stage. */
                _nc_movegen_gen_castles(pos, &state->moves);
                ++state->stage;
                return nc_movegen_next_move(pos, state, out);
            case NC_MOVEGEN_STAGE_QUIETS:
                /* Generate quiet moves and try again. */
                _nc_movegen_gen_quiets(pos, NC_PAWN, &state->moves);
                _nc_movegen_gen_quiets(pos, NC_QUEEN, &state->moves);
                _nc_movegen_gen_quiets(pos, NC_ROOK, &state->moves);
                _nc_movegen_gen_quiets(pos, NC_KNIGHT, &state->moves);
                _nc_movegen_gen_quiets(pos, NC_BISHOP, &state->moves);
                _nc_movegen_gen_quiets(pos, NC_KING, &state->moves);
                ++state->stage;
                return nc_movegen_next_move(pos, state, out);
            case NC_MOVEGEN_STAGE_END:
                /* No moves left. Break out! */
                return 0;
        }
    }

    return 0;
}

nc_bb nc_movegen_attacked_squares(nc_position* pos, nc_color col) {
    nc_bb out = 0;

    for (int t = 0; t <= 5; ++t) {
        nc_bb srcmask = pos->color[col] & pos->piece[t];

        if (!srcmask) continue;

        switch (t) {
        case NC_PAWN:
        {
            /* Generate all pawn attacks. */

            int left_dir = (col == NC_WHITE) ? NC_SQ_NORTHWEST : NC_SQ_SOUTHWEST;
            int right_dir = (col == NC_WHITE) ? NC_SQ_NORTHEAST : NC_SQ_SOUTHEAST;

            out |= nc_bb_shift(srcmask & ~NC_BB_FILEA, left_dir);
            out |= nc_bb_shift(srcmask & ~NC_BB_FILEH, right_dir);

            break;
        }
        case NC_KNIGHT:
            /* Generate knight attacks. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                out |= nc_basic_knight_attacks(src);
            }
            break;
        case NC_BISHOP:
            /* Generate bishop attacks. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                out |= nc_magic_query_bishop_attacks(src, pos->global);
            }
            break;
        case NC_ROOK:
            /* Generate rook attacks. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                out |= nc_magic_query_rook_attacks(src, pos->global);
            }
            break;
        case NC_QUEEN:
            /* Generate queen attacks. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                out |= nc_magic_query_rook_attacks(src, pos->global);
                out |= nc_magic_query_bishop_attacks(src, pos->global);
            }
            break;
        case NC_KING:
            /* Generate king attacks. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                out |= nc_basic_king_attacks(src);
            }
            break;
        }
    }

    return out;
}

int nc_movegen_square_is_attacked(nc_position* pos, nc_square sq, nc_color opp) {
    nc_bb oppmask = pos->color[opp];
    nc_bb sqmask = nc_bb_mask(sq);

    /* Test for sliding piece attacks */
    nc_bb bishop_attacks = nc_magic_query_bishop_attacks(sq, pos->global);
    if (bishop_attacks & ((pos->piece[NC_QUEEN] | pos->piece[NC_BISHOP]) & oppmask)) return 1;

    nc_bb rook_attacks = nc_magic_query_rook_attacks(sq, pos->global);
    if (rook_attacks & ((pos->piece[NC_QUEEN] | pos->piece[NC_ROOK]) & oppmask)) return 1;

    /* Test for knight attacks */
    if (nc_basic_knight_attacks(sq) & (oppmask & pos->piece[NC_KNIGHT])) return 1;

    /* Test for pawn attacks */
    nc_bb opp_pawns = pos->piece[NC_PAWN] & pos->color[opp];

    int left_dir = (opp == NC_BLACK) ? NC_SQ_NORTHWEST : NC_SQ_SOUTHWEST;
    if (nc_bb_shift(sqmask & ~NC_BB_FILEA, left_dir) & opp_pawns) return 1;

    int right_dir = (opp == NC_BLACK) ? NC_SQ_NORTHEAST : NC_SQ_SOUTHEAST;
    if (nc_bb_shift(sqmask & ~NC_BB_FILEH, right_dir) & opp_pawns) return 1;

    return 0;
}

int nc_movegen_mask_is_attacked(nc_position* pos, nc_bb mask, nc_color opp) {
    nc_bb oppmask = pos->color[opp], tmp;

    /* Test for sliding piece attacks */
    tmp = mask;
    while (tmp) {
        nc_bb bishop_attacks = nc_magic_query_bishop_attacks(nc_bb_poplsb(&tmp), pos->global);
        if (bishop_attacks & ((pos->piece[NC_QUEEN] | pos->piece[NC_BISHOP]) & oppmask)) return 1;
    }

    tmp = mask;
    while (tmp) {
        nc_bb rook_attacks = nc_magic_query_rook_attacks(nc_bb_poplsb(&tmp), pos->global);
        if (rook_attacks & ((pos->piece[NC_QUEEN] | pos->piece[NC_ROOK]) & oppmask)) return 1;
    }

    /* Test for knight attacks */
    tmp = mask;
    while (tmp) {
        if (nc_basic_knight_attacks(nc_bb_poplsb(&tmp)) & (oppmask & pos->piece[NC_KNIGHT])) return 1;
    }

    /* Test for pawn attacks */
    nc_bb opp_pawns = pos->piece[NC_PAWN] & pos->color[opp];

    int left_dir = (opp == NC_BLACK) ? NC_SQ_NORTHWEST : NC_SQ_SOUTHWEST;
    if (nc_bb_shift(mask & ~NC_BB_FILEA, left_dir) & opp_pawns) return 1;

    int right_dir = (opp == NC_BLACK) ? NC_SQ_NORTHEAST : NC_SQ_SOUTHEAST;
    if (nc_bb_shift(mask & ~NC_BB_FILEH, right_dir) & opp_pawns) return 1;

    return 0;
}

int nc_movegen_get_king_in_check(nc_position* pos, nc_color col) {
    return nc_movegen_square_is_attacked(pos, nc_bb_getlsb(pos->piece[NC_KING] & pos->color[col]), nc_colorflip(col));
}

void _nc_movegen_gen_castles(nc_position* pos, nc_movelist* out) {
    /* Early break if in check. */
    if (pos->states[pos->ply].check) return;

    int rights = pos->states[pos->ply].castling;
    int opp = nc_colorflip(pos->color_to_move);

    int qs = (pos->color_to_move == NC_WHITE) ? (rights & NC_WHITE_QUEENSIDE) : (rights & NC_BLACK_QUEENSIDE);
    int ks = (pos->color_to_move == NC_WHITE) ? (rights & NC_WHITE_KINGSIDE) : (rights & NC_BLACK_KINGSIDE);
    nc_bb rankmask = (pos->color_to_move == NC_WHITE) ? NC_BB_RANK1 : NC_BB_RANK8;
    int src = (pos->color_to_move == NC_WHITE) ? NC_SQ_E1 : NC_SQ_E8;

    /* Try queenside. */
    if (qs) {
        nc_bb badmask = NC_BB_QS_BADFILES & rankmask;
        nc_bb occmask = NC_BB_QS_OCCFILES & rankmask;

        if (!(pos->global & occmask) && !nc_movegen_mask_is_attacked(pos, badmask, opp)) {
            int dst = (pos->color_to_move == NC_WHITE) ? NC_SQ_A1 : NC_SQ_A8;

            nc_movelist_push(out, nc_move_make(src, dst) | NC_CASTLE);
        }
    }

    /* Then try kingside. */
    if (ks) {
        nc_bb badmask = NC_BB_KS_BADFILES & rankmask;
        nc_bb occmask = NC_BB_KS_OCCFILES & rankmask;

        if (!(pos->global & occmask) && !nc_movegen_mask_is_attacked(pos, badmask, opp)) {
            int dst = (pos->color_to_move == NC_WHITE) ? NC_SQ_H1 : NC_SQ_H8;

            nc_movelist_push(out, nc_move_make(src, dst) | NC_CASTLE);
        }
    }
}

void _nc_movegen_gen_quiets(nc_position* pos, int type, nc_movelist* out) {
    nc_bb srcmask = pos->color[pos->color_to_move] & pos->piece[type];

    if (!srcmask) return;

    switch (type) {
        case NC_PAWN:
        {
            /* Look at promoting advances first. */
            int dir = (pos->color_to_move == NC_WHITE) ? NC_SQ_NORTH : NC_SQ_SOUTH;

            nc_bb ppawns = (pos->color_to_move == NC_WHITE) ? (srcmask & NC_BB_RANK7) : (srcmask & NC_BB_RANK2);
            nc_bb spawns = (pos->color_to_move == NC_WHITE) ? (srcmask & NC_BB_RANK2) : (srcmask & NC_BB_RANK7);
            nc_bb npawns = srcmask ^ ppawns;

            /* note: ppawns and npawns are exclusive, but npawns contains spawns. */

            nc_bb advmask;

            if (ppawns) {
                advmask = nc_bb_shift(ppawns, dir) & ~pos->global;

                while (advmask) {
                    int dst = nc_bb_poplsb(&advmask);
                    int src = dst - dir;

                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst), NC_QUEEN));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst), NC_KNIGHT));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst), NC_ROOK));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst), NC_BISHOP));
                }
            }

            /* Look at pawn jumps next. */
            if (spawns) {
                advmask = nc_bb_shift(spawns, dir) & ~pos->global;
                advmask = nc_bb_shift(advmask, dir) & ~pos->global;

                while (advmask) {
                    int dst = nc_bb_poplsb(&advmask);
                    int src = dst - dir * 2;

                    nc_movelist_push(out, nc_move_make(src, dst) | NC_PAWNJUMP);
                }
            }

            /* Look at single nonpromoting advances. */
            if (npawns) {
                advmask = nc_bb_shift(npawns, dir) & ~pos->global;

                while (advmask) {
                    int dst = nc_bb_poplsb(&advmask);
                    int src = dst - dir;

                    nc_movelist_push(out, nc_move_make(src, dst));
                }
            }

            break;
        }
        case NC_KNIGHT:
            /* Generate quiet knight moves. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                nc_bb dstmask = nc_basic_knight_attacks(src) & ~pos->global;

                while (dstmask) {
                    nc_square dst = nc_bb_poplsb(&dstmask);
                    nc_movelist_push(out, nc_move_make(src, dst));
                }
            }
            break;
        case NC_BISHOP:
            /* Generate quiet bishop moves. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                nc_bb dstmask = nc_magic_query_bishop_attacks(src, pos->global) & ~pos->global;

                while (dstmask) {
                    nc_square dst = nc_bb_poplsb(&dstmask);
                    nc_movelist_push(out, nc_move_make(src, dst));
                }
            }
            break;
        case NC_ROOK:
            /* Generate quiet rook moves. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                nc_bb dstmask = nc_magic_query_rook_attacks(src, pos->global) & ~pos->global;

                while (dstmask) {
                    nc_square dst = nc_bb_poplsb(&dstmask);
                    nc_movelist_push(out, nc_move_make(src, dst));
                }
            }
            break;
        case NC_QUEEN:
            /* Generate quiet queen moves. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                nc_bb dstmask = nc_magic_query_rook_attacks(src, pos->global);

                dstmask |= nc_magic_query_bishop_attacks(src, pos->global);
                dstmask &= ~pos->global;

                while (dstmask) {
                    nc_square dst = nc_bb_poplsb(&dstmask);
                    nc_movelist_push(out, nc_move_make(src, dst));
                }
            }
            break;
        case NC_KING:
            /* Generate quiet king moves. */
            while (srcmask) {
                nc_square src = nc_bb_poplsb(&srcmask);
                nc_bb dstmask = nc_basic_king_attacks(src) & ~pos->global;

                while (dstmask) {
                    nc_square dst = nc_bb_poplsb(&dstmask);
                    nc_movelist_push(out, nc_move_make(src, dst));
                }
            }
            break;
    }
}

void _nc_movegen_gen_captures(nc_position* pos, int victim, int attacker, nc_movelist* out) {
    /* Grab moves from the attack masks. */
    int ctm = pos->color_to_move;
    int opp = nc_colorflip(pos->color_to_move);

    nc_bb vmask = pos->color[opp] & pos->piece[victim];
    nc_bb amask = pos->color[ctm] & pos->piece[attacker];

    /* Early exit if no captures are possible. */
    if (!vmask || !amask) return;

    switch (attacker) {
        case NC_PAWN:
        {
            /* Add en passant moves if PxP. */
            if (victim == NC_PAWN && pos->states[pos->ply].ep_target != NC_SQ_NULL) vmask |= nc_bb_mask(pos->states[pos->ply].ep_target);

            int left_dir = (ctm == NC_WHITE) ? NC_SQ_NORTHWEST : NC_SQ_SOUTHWEST;
            int right_dir = (ctm == NC_WHITE) ? NC_SQ_NORTHEAST : NC_SQ_SOUTHEAST;

            /* Do promoting captures first. */
            nc_bb ppawns = (ctm == NC_WHITE) ? (amask & NC_BB_RANK7) : (amask & NC_BB_RANK2);
            nc_bb npawns = amask ^ ppawns;

            nc_bb left_attacks, right_attacks;

            if (ppawns) {
                /* Add left promoting attacks */
                left_attacks = nc_bb_shift(ppawns & ~NC_BB_FILEA, left_dir) & vmask;

                while (left_attacks) {
                    nc_square dst = nc_bb_poplsb(&left_attacks);
                    nc_square src = dst - left_dir;

                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_QUEEN));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_KNIGHT));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_ROOK));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_BISHOP));
                }

                /* Add right promoting attacks */
                right_attacks = nc_bb_shift(ppawns & ~NC_BB_FILEH, right_dir) & vmask;

                while (right_attacks) {
                    nc_square dst = nc_bb_poplsb(&right_attacks);
                    nc_square src = dst - right_dir;

                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_QUEEN));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_KNIGHT));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_ROOK));
                    nc_movelist_push(out, nc_move_promotion(nc_move_make(src, dst) | NC_CAPTURE, NC_BISHOP));
                }
            }

            /* Add left attacks */
            left_attacks = nc_bb_shift(npawns & ~NC_BB_FILEA, left_dir) & vmask;

            while (left_attacks) {
                nc_square dst = nc_bb_poplsb(&left_attacks);
                nc_square src = dst - left_dir;

                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }

            /* Add right attacks */
            right_attacks = nc_bb_shift(npawns & ~NC_BB_FILEH, right_dir) & vmask;

            while (right_attacks) {
                nc_square dst = nc_bb_poplsb(&right_attacks);
                nc_square src = dst - right_dir;

                nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
            }

            break;
        }
        case NC_KNIGHT:
            /* Generate knight attack masks. */
            while (amask) {
                nc_square src = nc_bb_poplsb(&amask);
                nc_bb attacks = nc_basic_knight_attacks(src) & vmask;

                while (attacks) {
                    nc_square dst = nc_bb_poplsb(&attacks);
                    nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
                }
            }
            break;
        case NC_BISHOP:
            /* Generate bishop captures. */
            while (amask) {
                nc_square src = nc_bb_poplsb(&amask);
                nc_bb attacks = nc_magic_query_bishop_attacks(src, pos->global) & vmask;

                while (attacks) {
                    nc_square dst = nc_bb_poplsb(&attacks);
                    nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
                }
            }
            break;
        case NC_ROOK:
            /* Generate rook captures. */
            while (amask) {
                nc_square src = nc_bb_poplsb(&amask);
                nc_bb attacks = nc_magic_query_rook_attacks(src, pos->global) & vmask;

                while (attacks) {
                    nc_square dst = nc_bb_poplsb(&attacks);
                    nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
                }
            }
            break;
        case NC_QUEEN:
            /* Generate queen captures. */
            while (amask) {
                nc_square src = nc_bb_poplsb(&amask);
                nc_bb attacks = nc_magic_query_rook_attacks(src, pos->global);

                attacks |= nc_magic_query_bishop_attacks(src, pos->global);
                attacks &= vmask;

                while (attacks) {
                    nc_square dst = nc_bb_poplsb(&attacks);
                    nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
                }
            }
            break;
        case NC_KING:
            /* Generate king captures. */
            while (amask) {
                nc_square src = nc_bb_poplsb(&amask);
                nc_bb attacks = nc_basic_king_attacks(src) & vmask;

                while (attacks) {
                    nc_square dst = nc_bb_poplsb(&attacks);
                    nc_movelist_push(out, nc_move_make(src, dst) | NC_CAPTURE);
                }
            }
            break;
    }
}
