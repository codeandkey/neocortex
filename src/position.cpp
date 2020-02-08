#include "position.h"

#include "lookup_pawn.h"
#include "lookup_knight.h"
#include "lookup_bishop.h"
#include "lookup_rook.h"
#include "lookup_king.h"
#include "tindex.h"
#include "piece.h"
#include "square.h"
#include "eval.h"
#include "ttable.h"

#include <iostream>

using namespace nc2;

static const char _nc2_position_init_uci[] = "RNBQKBNRPPPPPPPP8888pppppppprnbqkbnr";

static const u64 _nc2_position_castle_noattack_masks[2][2] = {
    { square::MASK_E1 | square::MASK_D1 | square::MASK_C1, square::MASK_E1 | square::MASK_F1 | square::MASK_G1 },
    { square::MASK_E8 | square::MASK_D8 | square::MASK_C8, square::MASK_E8 | square::MASK_F8 | square::MASK_G8 },
};

Position::Position() {
    ttable_index = 0;

    /* Initialize board pieces. */
    int ind = 0;
    for (u8 i = 0; i < sizeof _nc2_position_init_uci - 1; ++i) {
        for (auto p : piece::from_uci(_nc2_position_init_uci[i])) {
            board[ind] = p;

            if (piece::exists(p)) {
                ttable_index ^= ttable::get_piece_key(ind, p);
            }

            ++ind;
        }
    }

    /* Initialize king masks, although not really needed. */
    king_masks[piece::Color::WHITE] = square::MASK_E1;
    king_masks[piece::Color::BLACK] = square::MASK_E8;

    /* Initialize check states -- no color is in check. */
    check_states[piece::Color::WHITE] = false;
    check_states[piece::Color::BLACK] = false;

    /* Enable castling for both sides. */
    castle_states[piece::Color::WHITE][0] = true;
    castle_states[piece::Color::WHITE][1] = true;
    castle_states[piece::Color::BLACK][0] = true;
    castle_states[piece::Color::BLACK][1] = true;

    /* Update transposition table index. */
    ttable_index ^= ttable::get_castle_key(piece::Color::WHITE, 0);
    ttable_index ^= ttable::get_castle_key(piece::Color::WHITE, 1);
    ttable_index ^= ttable::get_castle_key(piece::Color::BLACK, 0);
    ttable_index ^= ttable::get_castle_key(piece::Color::BLACK, 1);

    /* Initialize move numbers and halfmove clock */
    halfmove_clock = 0;
    fullmove_number = 1;

    /* White to move */
    color_to_move = piece::Color::WHITE;

    /* Initial position is quiet */
    quiet = true;

    /* Initialize occtables */
    global_occ = Occboard::standard();

    for (int f = 0; f < 8; ++f) {
        color_occ[piece::Color::WHITE].flip(square::at(0, f));
        color_occ[piece::Color::WHITE].flip(square::at(1, f));

        color_occ[piece::Color::BLACK].flip(square::at(6, f));
        color_occ[piece::Color::BLACK].flip(square::at(7, f));
    }

    /* Initialize attack masks */
    update_check_states();

    /* Initialize en passant target */
    en_passant_target = square::null;

    /* Not evaluated by default. */
    computed_eval = false;
}

Position::Position(std::string fen) {
    throw std::runtime_error("FEN parsing not implemented!");
}

std::vector<Position::Transition> Position::gen_legal_moves() {
    std::vector<Position::Transition> pl_moves = gen_pseudolegal_moves();

    //std::cerr << "Generating legal moves for position..\n";

    auto it = pl_moves.begin();

    while (it != pl_moves.end()) {
        if (!(*it).second.update_check_states()) {
            /* Illegal position! Drop this move. */
            //std::cerr << "Rejecting " << (*it).first.to_string() << "\n";
            //std::cerr << "^ due to king_mask = \n" << bitboard_to_string((*it).second.king_masks[color_to_move]);
            //std::cerr << "^ due to (other) attack_mask = \n" << bitboard_to_string((*it).second.attack_masks[piece::colorflip(color_to_move)]);
            it = pl_moves.erase(it);
        } else {
            //std::cerr << "Accepting " << (*it).first.to_string() << "\n";
            (*it).second.computed_eval = false; // force the resulting position to be re-evaluated
            ++it;
        }
    }

    //std::cerr << "Done generating legal moves.\n";

    return pl_moves;
}

std::string Position::get_debug_string() {
    /* Print out board UCI pieces, as well as the global occboard and attack masks. */
    std::string out;

    out += "board state: transposition hash " + std::to_string(ttable_index) + "\n";
    if (check_states[piece::Color::WHITE]) out += "white in CHECK\n";
    if (check_states[piece::Color::BLACK]) out += "black in CHECK\n";
    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            u8 p = board[square::at(r, f)];

            if (piece::exists(p)) {
                out += piece::uci(p);
            } else {
                out += '.';
            }
        }

        out += '\n';
    }

    out += "white attacks:\n";
    out += bitboard_to_string(attack_masks[piece::Color::WHITE]);

    out += "black attacks:\n";
    out += bitboard_to_string(attack_masks[piece::Color::BLACK]);

    out += "global occ:\n";
    out += bitboard_to_string(global_occ.get_board());

    out += "white king mask:\n";
    out += bitboard_to_string(king_masks[piece::Color::WHITE]);

    out += "black king mask:\n";
    out += bitboard_to_string(king_masks[piece::Color::BLACK]);

    return out;
}

u64 Position::get_ttable_key() {
    return ttable_index;
}

u8* Position::get_board() {
    return board;
}

void Position::compute_eval() {
    computed_eval = true;

    float current_eval_value = 0.0f;
    current_eval_value = eval::evaluate(board, attack_masks[piece::Color::WHITE], attack_masks[piece::Color::BLACK]);

    if (color_to_move == piece::Color::WHITE) {
        current_eval_value += eval::TEMPO_VALUE;
    } else {
        current_eval_value -= eval::TEMPO_VALUE;
    }

    current_eval = current_eval_value + eval::noise();
}

float Position::get_eval_heuristic() {
    if (!computed_eval) compute_eval();

    return current_eval;
}

bool Position::is_quiet() {
    return quiet;
}

u8 Position::get_color_to_move() {
    return color_to_move;
}

bool Position::get_color_in_check(u8 col) {
    return check_states[col];
}

std::vector<Position::Transition> Position::gen_pseudolegal_moves() {
    std::vector<Position::Transition> output = gen_castle_moves();

    for (u8 s = 0; s < 64; ++s) {
        if (board[s] == piece::null) continue;

        u8 c = piece::color(board[s]);
        if (c != color_to_move) continue;

        u8 t = piece::type(board[s]);
        switch (t) {
            case piece::Type::PAWN:
                filter_pawn_captures(lookup::pawn_captures(s, c), &output);
                filter_pawn_advances(lookup::pawn_advances(s, c), &output);
                break;
            case piece::Type::KNIGHT:
                filter_basic_moves(lookup::knight_moves(s), &output);
                break;
            case piece::Type::BISHOP:
                filter_basic_moves(lookup::bishop_moves(s, &global_occ), &output);
                break;
            case piece::Type::ROOK:
                filter_basic_moves(lookup::rook_moves(s, &global_occ), &output);
                break;
            case piece::Type::KING:
                filter_basic_moves(lookup::king_moves(s), &output);
                break;
            case piece::Type::QUEEN:
                filter_basic_moves(lookup::rook_moves(s, &global_occ), &output);
                filter_basic_moves(lookup::bishop_moves(s, &global_occ), &output);
                break;
        }
    }

    return output;
}

void Position::filter_basic_moves(const std::vector<Move>& source, std::vector<Position::Transition>* output) {
    /* Filter basic moves and generate the resulting board state. */

    for (auto m : source) {
        u8 from = m.get_from(), to = m.get_to();

        /* Can't capture own pieces */
        if (piece::exists(board[to]) && piece::color(board[to]) == color_to_move) continue;

        /* Save if the move is a capture. */
        bool is_capture = piece::exists(board[to]);

        /* Build resulting state now. */
        Position result = *this;

        /* Update result ttable_index. */
        if (is_capture) {
            result.ttable_index ^= ttable::get_piece_key(to, board[to]); /* remove to piece */
        }

        result.ttable_index ^= ttable::get_piece_key(from, board[from]); /* remove from piece */
        result.ttable_index ^= ttable::get_piece_key(to, board[from]); /* place new piece */

        /* Move is not quiet if it was a capture */
        result.quiet = !is_capture;

        /* Update board values. */
        result.board[to] = board[from];
        result.board[from] = piece::null;

        /* Update color to move */
        result.color_to_move = piece::colorflip(color_to_move);
        result.ttable_index ^= ttable::get_black_to_move_key();

        /* Update move numbers */
        if (color_to_move == piece::Color::BLACK) {
            ++result.fullmove_number;
        }

        if (!result.quiet) {
            result.halfmove_clock = 0;
        } else {
            ++result.halfmove_clock;
        }

        /* Update king masks if needed. */
        if (piece::type(board[from]) == piece::Type::KING) {
            result.king_masks[color_to_move] = square::mask(to);

            /* King moves remove castle rights */
            result.castle_states[color_to_move][0] = false;
            result.castle_states[color_to_move][1] = false;
        }

        /* Check for castle breaking */
        if (from == square::Squares::a1 || to == square::Squares::a1) {
            result.castle_states[piece::Color::WHITE][0] = false;
        }

        if (from == square::Squares::h1 || to == square::Squares::h1) {
            result.castle_states[piece::Color::WHITE][1] = false;
        }

        if (from == square::Squares::a8 || to == square::Squares::a8) {
            result.castle_states[piece::Color::BLACK][0] = false;
        }

        if (from == square::Squares::h8 || to == square::Squares::h8) {
            result.castle_states[piece::Color::BLACK][1] = false;
        }

        /* Check for difference in castle states and update ttable_index accordingly. */
        for (u8 c = 0; c < 2; ++c) {
            for (u8 side = 0; side < 2; ++side) {
                if (result.castle_states[c][side] != castle_states[c][side]) {
                    result.ttable_index ^= ttable::get_castle_key(c, side);
                }
            }
        }

        /* Unset the en passant target and update the tindex if needed. */
        if (en_passant_target != square::null) {
            result.ttable_index ^= ttable::get_en_passant_file_key(square::file(en_passant_target));
            result.en_passant_target = square::null;
        }

        /* Update the global occupancy and color occupancy tables. */
        result.global_occ.flip(from);
        result.color_occ[color_to_move].flip(from);
        result.color_occ[color_to_move].flip(to);

        if (is_capture) {
            result.color_occ[piece::colorflip(color_to_move)].flip(to);
        } else {
            result.global_occ.flip(to);
        }

        /* Set the result check states to false (they will be updated later anyway). */
        result.check_states[piece::Color::WHITE] = false;
        result.check_states[piece::Color::BLACK] = false;

        /* Finally, push the resulting position to the output list. */
        output->push_back(Transition(m, result));
    }
}

void Position::filter_pawn_advances(const std::vector<Move>& source, std::vector<Position::Transition>* output) {
    /* Filter pawn advances and generate the resulting board state. */

    for (auto m : source) {
        u8 from = m.get_from(), to = m.get_to();

        u8 dst_type = m.get_ptype();

        if (dst_type == piece::Type::NONE) {
            dst_type = piece::Type::PAWN;
        }

        u8 dst_piece = piece::make(dst_type, color_to_move);

        int rdiff = ((int) square::rank(from) - (int) square::rank(to));
        bool is_jump = (rdiff == 2 || rdiff == -2);

        /* If jump, check the occupancy mask. */
        if (is_jump && !global_occ.pawn_can_jump(square::file(from), color_to_move)) continue;

        /* Can't move onto an occupied square. */
        if (piece::exists(board[to])) continue;

        /* Build resulting state now. */
        Position result = *this;

        /* Update result ttable_index. */
        result.ttable_index ^= ttable::get_piece_key(from, board[from]); /* remove from piece */
        result.ttable_index ^= ttable::get_piece_key(to, dst_piece); /* place new piece */

        /* Pawn advances are quiet */
        result.quiet = true;

        /* Update board values. */
        result.board[to] = dst_piece;
        result.board[from] = piece::null;

        /* Update color to move */
        result.color_to_move = piece::colorflip(color_to_move);
        result.ttable_index ^= ttable::get_black_to_move_key();

        /* Update move numbers */
        if (color_to_move == piece::Color::BLACK) {
            ++result.fullmove_number;
        }

        result.halfmove_clock = 0;

        /* Update en passant target if needed and update tindex. */
        if (en_passant_target != square::null) {
            result.ttable_index ^= ttable::get_en_passant_file_key(square::file(en_passant_target));
            result.en_passant_target = square::null;
        }

        if (is_jump) {
            result.en_passant_target = square::at((color_to_move == piece::Color::WHITE) ? 2 : 5, square::file(to));
            result.ttable_index ^= ttable::get_en_passant_file_key(square::file(to));
        }

        /* Update the global occupancy and color occupancy tables. */
        result.global_occ.flip(from);
        result.global_occ.flip(to);
        result.color_occ[color_to_move].flip(from);
        result.color_occ[color_to_move].flip(to);

        /* Set the result check states to false (they will be updated later anyway). */
        result.check_states[piece::Color::WHITE] = false;
        result.check_states[piece::Color::BLACK] = false;

        /* Finally, push the resulting position to the output list. */
        output->push_back(Transition(m, result));
    }
}

void Position::filter_pawn_captures(const std::vector<Move>& source, std::vector<Position::Transition>* output) {
    /* Filter pawn captures and generate the resulting board state. */

    for (auto m : source) {
        u8 from = m.get_from(), to = m.get_to();

        bool en_passant = false;
        u8 en_passant_piece = 0;

        u8 dst_type = m.get_ptype();

        if (dst_type == piece::Type::NONE) {
            dst_type = piece::Type::PAWN;
        }

        u8 dst_piece = piece::make(dst_type, color_to_move);

        /* Can't capture own pieces */
        if (piece::exists(board[to])) {
            if (piece::color(board[to]) == color_to_move) continue;
        } else {
            if (to != en_passant_target) continue;
            en_passant = true;
            en_passant_piece = square::at((color_to_move == piece::Color::WHITE) ? 4 : 3, square::file(en_passant_target));
        }

        /* Build resulting state now. */
        Position result = *this;

        /* Update result ttable_index. */
        result.ttable_index ^= ttable::get_piece_key(from, board[from]); /* remove from piece */
        result.ttable_index ^= ttable::get_piece_key(to, dst_piece); /* place new piece */

        if (en_passant) {
            result.ttable_index ^= ttable::get_piece_key(en_passant_piece, board[en_passant_piece]); /* remove captured EP piece */
        } else {
            result.ttable_index ^= ttable::get_piece_key(to, board[to]);
        }

        /* Move is not quiet, all are captures */
        result.quiet = false;

        /* Update board values. */
        result.board[to] = dst_piece;
        result.board[from] = piece::null;

        if (en_passant) {
            result.board[en_passant_piece] = piece::null;
        }

        /* Update color to move */
        result.color_to_move = piece::colorflip(color_to_move);
        result.ttable_index ^= ttable::get_black_to_move_key();

        /* Update move numbers */
        if (color_to_move == piece::Color::BLACK) {
            ++result.fullmove_number;
        }

        ++result.halfmove_clock;

        /* Check for castle breaking */
        if (to == square::Squares::a1) {
            result.castle_states[piece::Color::WHITE][0] = false;
        }

        if (to == square::Squares::h1) {
            result.castle_states[piece::Color::WHITE][1] = false;
        }

        if (to == square::Squares::a8) {
            result.castle_states[piece::Color::BLACK][0] = false;
        }

        if (to == square::Squares::h8) {
            result.castle_states[piece::Color::BLACK][1] = false;
        }

        /* Check for difference in castle states and update ttable_index accordingly. */
        for (u8 c = 0; c < 2; ++c) {
            for (u8 side = 0; side < 2; ++side) {
                if (result.castle_states[c][side] != castle_states[c][side]) {
                    result.ttable_index ^= ttable::get_castle_key(c, side);
                }
            }
        }

        /* Unset the en passant target and update the tindex if needed. */
        if (en_passant_target != square::null) {
            result.ttable_index ^= ttable::get_en_passant_file_key(square::file(en_passant_target));
            result.en_passant_target = square::null;
        }

        /* Update the global occupancy and color occupancy tables. */
        result.global_occ.flip(from);
        result.color_occ[color_to_move].flip(from);
        result.color_occ[color_to_move].flip(to);

        if (en_passant) {
            result.color_occ[piece::colorflip(color_to_move)].flip(en_passant_piece);
            result.global_occ.flip(en_passant_piece);
            result.global_occ.flip(to);
        } else {
            result.color_occ[piece::colorflip(color_to_move)].flip(to);
        }

        /* Set the result check states to false (they will be updated later anyway). */
        result.check_states[piece::Color::WHITE] = false;
        result.check_states[piece::Color::BLACK] = false;

        /* Finally, push the resulting position to the output list. */
        output->push_back(Transition(m, result));
    }
}

std::vector<Position::Transition> Position::gen_castle_moves() {
    std::vector<Position::Transition> output;
    u8 dst_rank = (color_to_move == piece::Color::WHITE) ? 0 : 7;

    for (int side = 0; side < 2; ++side) {
        if (castle_states[color_to_move][side]) {
            //std::cerr << "castle_states[" << (int) color_to_move << "][" << side << "] ok\n";
            if (!(attack_masks[piece::colorflip(color_to_move)] & _nc2_position_castle_noattack_masks[color_to_move][side])) {
                //std::cerr << "attack_mask OK\n";
                if (global_occ.color_can_castle(color_to_move, side)) {
                    //std::cerr << "global_occ test OK\n";
                    u8 from, to, rfrom, rto;

                    if (side == 0) {
                        /* Queenside target squares */
                        from = square::at(dst_rank, 4);
                        to = square::at(dst_rank, 2);
                        rfrom = square::at(dst_rank, 0);
                        rto = square::at(dst_rank, 3);
                    } else {
                        /* Kingside target squares */
                        from = square::at(dst_rank, 4);
                        to = square::at(dst_rank, 6);
                        rfrom = square::at(dst_rank, 7);
                        rto = square::at(dst_rank, 5);
                    }

                    /* Castle move is OK ! Create the resulting state. */
                    Position result = *this;

                    /* Update ttable_index pieces */
                    result.ttable_index ^= ttable::get_piece_key(rfrom, board[rfrom]); /* remove rook */
                    result.ttable_index ^= ttable::get_piece_key(rto, board[rfrom]); /* place rook */
                    result.ttable_index ^= ttable::get_piece_key(from, board[from]); /* remove king */
                    result.ttable_index ^= ttable::get_piece_key(to, board[from]); /* place king */

                    /* Update resulting board state */
                    result.board[rto] = result.board[rfrom];
                    result.board[rfrom] = piece::null;
                    result.board[to] = result.board[from];
                    result.board[from] = piece::null;

                    /* Update new castle states, update index difference */
                    for (int s = 0; s < 2; ++s) {
                        result.castle_states[color_to_move][s] = false;

                        if (result.castle_states[color_to_move][s] != castle_states[color_to_move][s]) {
                            result.ttable_index ^= ttable::get_castle_key(color_to_move, s);
                        }
                    }

                    /* Update result halfmove clock, move number */
                    ++result.halfmove_clock;

                    if (color_to_move == piece::Color::BLACK) {
                        ++result.fullmove_number;
                    }

                    /* Update resulting color to move */
                    result.color_to_move = piece::colorflip(color_to_move);
                    result.ttable_index ^= ttable::get_black_to_move_key();

                    /* Update resulting king mask */
                    result.king_masks[color_to_move] = square::mask(to);

                    /* Update resulting en passant target, ttable index */
                    if (en_passant_target != square::null) {
                        result.ttable_index ^= ttable::get_en_passant_file_key(square::file(en_passant_target));
                        result.en_passant_target = square::null;
                    }

                    /* Update global, color occupancy tables */
                    result.global_occ.flip(from);
                    result.global_occ.flip(to);
                    result.global_occ.flip(rfrom);
                    result.global_occ.flip(rto);

                    result.color_occ[color_to_move].flip(from);
                    result.color_occ[color_to_move].flip(to);
                    result.color_occ[color_to_move].flip(rfrom);
                    result.color_occ[color_to_move].flip(rto);

                    /* All castle moves are quiet (unless delivering check, tested for later) */
                    result.quiet = true;

                    output.push_back(Transition(Move(from, to), result));
                } else {
                    //std::cerr << "global_occ failed\n";
                }
            } else {
                //std::cerr << "attack mask failed:\nattack_mask=\n";
                //std::cerr << bitboard_to_string(attack_masks[piece::colorflip(color_to_move)]) << "\nnoattackmask=\n";
                //std::cerr << bitboard_to_string(_nc2_position_castle_noattack_masks[color_to_move][side]) << "\n";
            }
        } else {
            //std::cerr << "castle_states[" << (int) color_to_move << "][" << side << "] failed\n";
        }
    }

    return output;
}

bool Position::update_check_states() {
    /* Generate attack masks for both colors. */

    attack_masks[piece::Color::WHITE] = 0;
    attack_masks[piece::Color::BLACK] = 0;

    for (u8 s = 0; s < 64; ++s) {
        u8 p = board[s];

        if (!piece::exists(p)) continue;

        u8 c = piece::color(p);

        switch (piece::type(p)) {
        case piece::Type::PAWN:
            attack_masks[c] |= lookup::pawn_attacks(s, c);
            break;
        case piece::Type::KNIGHT:
            attack_masks[c] |= lookup::knight_attacks(s);
            break;
        case piece::Type::ROOK:
            attack_masks[c] |= lookup::rook_attacks(s, &global_occ);
            break;
        case piece::Type::BISHOP:
            attack_masks[c] |= lookup::bishop_attacks(s, &global_occ);
            break;
        case piece::Type::QUEEN:
            attack_masks[c] |= lookup::bishop_attacks(s, &global_occ);
            attack_masks[c] |= lookup::rook_attacks(s, &global_occ);
            break;
        case piece::Type::KING:
            attack_masks[c] |= lookup::king_attacks(s);
            break;
        }
    }

    check_states[piece::Color::WHITE] = attack_masks[piece::Color::BLACK] & king_masks[piece::Color::WHITE];
    check_states[piece::Color::BLACK] = attack_masks[piece::Color::WHITE] & king_masks[piece::Color::BLACK];

    if (check_states[piece::Color::WHITE] || check_states[piece::Color::BLACK]) quiet = false;

    /* The color that just moved cannot be in check. */
    return !check_states[piece::colorflip(color_to_move)];
}
