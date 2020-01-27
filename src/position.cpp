#include "position.h"
#include "lookup.h"
#include "log.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace nc;

Position::Position() {
    /* Standard game init */
    white_in_check = black_in_check = false;
    w_kingside = w_queenside = b_kingside = b_queenside = true;
    color_to_move = 'w';

    /* We don't actually need to set the king masks as there are no possible checks on the first move. */
    white_king_mask = ((u64) 1 << 4);
    black_king_mask = ((u64) 1 << 60);

    quiet = false;

    compute_attack_masks();

    halfmove_clock = 0;
    fullmove_number = 1;

    occ = Occtable::standard();

    for (int f = 0; f < 8; ++f) {
        board[Square(1, f).get_index()] = 'P';
        board[Square(6, f).get_index()] = 'p';
    }

    board[63] = 'r';
    board[62] = 'n';
    board[61] = 'b';
    board[60] = 'k';
    board[59] = 'q';
    board[58] = 'b';
    board[57] = 'n';
    board[56] = 'r';

    board[0] = 'R';
    board[1] = 'N';
    board[2] = 'B';
    board[3] = 'Q';
    board[4] = 'K';
    board[5] = 'B';
    board[6] = 'N';
    board[7] = 'R';
}

Position::Position(std::string fen) {
    /* Try and parse a valid FEN */

    std::stringstream ss(fen);

    std::string rinfo, col_to_move, castle, en_passant, halfmove, fullmove;

    ss >> rinfo;
    ss >> col_to_move;
    ss >> castle;
    ss >> en_passant;
    ss >> halfmove_clock;
    ss >> fullmove_number;

    color_to_move = col_to_move[0];

    int crank = 7, cfile = 0;
    for (auto c : rinfo) {
        if (c == '/') {
            crank--;
            cfile = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            int count = c - '0';

            for (int i = 0; i < count; ++i) {
                board[Square(crank, cfile++).get_index()] = Piece();
            }
        } else {
            board[Square(crank, cfile++).get_index()] = c;
        }
    }

    en_passant_target = en_passant;

    w_kingside = w_queenside = b_kingside = b_queenside = false;

    for (auto c : castle) {
        switch (c) {
            case 'K':
                w_kingside = true;
                break;
            case 'Q':
                w_queenside = true;
                break;
            case 'k':
                b_kingside = true;
                break;
            case 'q':
                b_queenside = true;
                break;
        }
    }

    /* initialize occtable */
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            if (board[Square(r, f).get_index()].is_valid()) {
                occ.flip(Square(r, f));

                /* Also set the king masks here. */
                if (board[Square(r, f).get_index()].get_uci() == 'K') {
                    white_king_mask = ((u64) 1 << Square(r, f).get_index());
                }

                if (board[Square(r, f).get_index()].get_uci() == 'k') {
                    black_king_mask = ((u64) 1 << Square(r, f).get_index());
                }
            }
        }
    }

    /* make masks, set quiet */
    quiet = true;
    compute_attack_masks();
}

std::string Position::get_fen() {
    std::string out;

    /* Write board state */
    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            Piece* p = board + Square(r, f).get_index();

            if (p->is_valid()) {
                out += p->get_uci();
            } else {
                /* Count empty squares */
                int count = 1;

                for (int k = f + 1; k < 8; ++k) {
                    if (board[Square(r, k).get_index()].is_valid()) break;
                    ++count;
                }

                out += std::to_string(count);
                f += (count - 1);
            }
        }

        if (r) out += '/';
    }

    /* Write color to move */
    out += ' ';
    out += color_to_move;

    /* Write castle state */
    out += ' ';

    if (w_kingside || w_queenside || b_kingside || b_queenside) {
        if (w_kingside) out += 'K';
        if (w_queenside) out += 'Q';
        if (b_kingside) out += 'k';
        if (b_queenside) out += 'q';
    } else {
        out += '-';
    }

    /* Write en passant target */
    out += ' ';
    out += en_passant_target.to_string();

    /* Write halfmove clock */
    out += ' ';
    out += std::to_string(halfmove_clock);

    /* Write fullmove number */
    out += ' ';
    out += std::to_string(fullmove_number);

    return out;
}

std::list<Position::Transition> Position::get_legal_moves() {
    /* First, generate all pseudolegal moves. */
    std::list<Position::Transition> out = get_pseudolegal_moves();

    /* Also get castling moves. */
    get_castle_moves(&out);

    /* Then, prune out those which leave the moving color in check. */
    auto psl_iter = out.begin();

    while (psl_iter != out.end()) {
        Position* result = (*psl_iter).get_result();

        result->compute_attack_masks();

        /* Check if the color that just moved is in check. */
        bool bad_check = result->get_color_in_check(color_to_move);

        if (bad_check) {
            /* Illegal move! Get out of here. */
            out.erase(psl_iter++);
        } else {
            /* Check if the move delivers check. */
            if (result->get_color_in_check(result->color_to_move)) {
                (*psl_iter).set_check(true);

                /* Check if the move delivers mate now. This might be unnecessary depending on how we do searching. */
                if (!result->get_legal_moves().size()) {
                    (*psl_iter).set_mate(true);
                }
            }

            psl_iter++;
        }
    }

    return out;
}

std::list<Position::Transition> Position::get_pseudolegal_moves() {
    std::list<Position::Transition> out;

    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            Piece* p = board + Square(r, f).get_index();

            if (p->is_valid() && p->get_color() == color_to_move) {
                switch (p->get_type()) {
                case 'k':
                    /* walk in a circle */
                    for (int x = -1; x <= 1; ++x) {
                        for (int y = -1; y <= 1; ++y) {
                            Square dst = Square(r+y,f+x);

                            if (!dst.is_valid()) continue;

                            /* Can't move on own pieces */
                            if (board[dst.get_index()].is_valid() && board[dst.get_index()].get_color() == color_to_move) {
                                continue;
                            }

                            /* Create the move! */
                            out.push_back(make_basic_pseudolegal_move(Square(r, f), dst));
                        }
                    }
                    break;
                case 'q':
                    get_pseudolegal_rook_moves(Square(r, f), &out);
                    get_pseudolegal_bishop_moves(Square(r, f), &out);
                    break;
                case 'r':
                    get_pseudolegal_rook_moves(Square(r, f), &out);
                    break;
                case 'b':
                    get_pseudolegal_bishop_moves(Square(r, f), &out);
                    break;
                case 'n':
                    for (int a = -1; a <= 1; a += 2) {
                        for (int b = -2; b <= 2; b += 4) {
                            Square first_dst(r + a, f + b);

                            if (first_dst.is_valid()) {
                                if (!(board[first_dst.get_index()].is_valid() && board[first_dst.get_index()].get_color() == color_to_move)) {
                                    out.push_back(make_basic_pseudolegal_move(Square(r, f), first_dst));
                                }
                            }

                            Square second_dst(r + b, f + a);

                            if (second_dst.is_valid()) {
                                if (!(board[second_dst.get_index()].is_valid() && board[second_dst.get_index()].get_color() == color_to_move)) {
                                    out.push_back(make_basic_pseudolegal_move(Square(r, f), second_dst));
                                }
                            }
                        }
                    }
                    break;
                case 'p':
                    {
                        /* Grab direction to save some code space */
                        int rdir = (color_to_move == 'w' ? 1 : -1);
                        int last = (color_to_move == 'w' ? 7 : 0);
                        int first = (color_to_move == 'w' ? 1 : 6);

                        /* Check for normal advances */
                        Square single_dst(r + rdir, f);

                        if (single_dst.is_valid() && !board[single_dst.get_index()].is_valid()) {
                            if (single_dst.get_rank() == last) {
                                /* Perform promotion if needed */
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), single_dst, 'b'));
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), single_dst, 'n'));
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), single_dst, 'r'));
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), single_dst, 'q'));
                            } else {
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), single_dst));
                            }
                        }

                        /* Check for double moves if allowed */
                        if (r == first) {
                            Square ep_dst(r + rdir, f);
                            Square double_dst(r + 2 * rdir, f);

                            if (!board[ep_dst.get_index()].is_valid() && !board[double_dst.get_index()].is_valid()) {
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), double_dst));
                            }
                        }

                        /* Check for captures */
                        for (int x = -1; x <= 1; x += 2) {
                            Square capture_dst(r + rdir, f + x);

                            if (!capture_dst.is_valid()) continue;

                            if (capture_dst != en_passant_target) {
                                if (!board[capture_dst.get_index()].is_valid() || board[capture_dst.get_index()].get_color() == color_to_move) {
                                    continue;
                                }
                            }

                            if (capture_dst.get_rank() == last) {
                                /* Perform promotion if needed */
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), capture_dst, 'b'));
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), capture_dst, 'n'));
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), capture_dst, 'r'));
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), capture_dst, 'q'));
                            } else {
                                out.push_back(make_basic_pseudolegal_move(Square(r, f), capture_dst));
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    return out;
}

void Position::get_castle_moves(std::list<Position::Transition>* out) {
    if (color_to_move == 'w') {
        Square from(0, 4);

        if (w_kingside) {
            u64 no_attack_mask = 0;

            no_attack_mask |= ((u64) 1 << Square(0, 4).get_index());
            no_attack_mask |= ((u64) 1 << Square(0, 5).get_index());
            no_attack_mask |= ((u64) 1 << Square(0, 6).get_index());

            /* Check the rook is in the corner. */
            if (board[Square(0, 7).get_index()].get_uci() == 'R') {
                /* Check the 3 target squares are not in check. */
                if (!(black_attack_mask & no_attack_mask)) {
                    /* Check the squares bwteen the king and rook are empty. */
                    if (!(occ.get_rank(0) & 0x60)) {
                        /* Make the move! */
                        Position result(*this);
                        Square to(0, 6);
                        Move move(from, to);

                        /* Unset castle flags */
                        result.w_kingside = result.w_queenside = false;

                        /* Move king, rook */
                        result.board[from.get_index()] = Piece();
                        result.board[to.get_index()] = 'K';
                        result.board[Square(0, 5).get_index()] = 'R';
                        result.board[Square(0, 7).get_index()] = Piece();

                        result.en_passant_target = Square();
                        result.color_to_move = colorflip(result.color_to_move);

                        result.occ.flip(from);
                        result.occ.flip(to);
                        result.occ.flip(Square(0, 5));
                        result.occ.flip(Square(0, 7));

                        result.white_king_mask = ((u64) 1 << to.get_index());

                        out->push_back(Transition(move, result));
                    }
                }
            }
        }

        if (w_queenside) {
            u64 no_attack_mask = 0;

            no_attack_mask |= ((u64) 1 << Square(0, 2).get_index());
            no_attack_mask |= ((u64) 1 << Square(0, 3).get_index());
            no_attack_mask |= ((u64) 1 << Square(0, 4).get_index());

            /* Check the rook is in the corner. */
            if (board[Square(0, 0).get_index()].get_uci() == 'R') {
                /* Check the 3 target squares are not in check. */
                if (!(black_attack_mask & no_attack_mask)) {
                    /* Check the squares bwteen the king and rook are empty. */
                    if (!(occ.get_rank(0) & 0x0E)) {
                        /* Make the move! */
                        Position result(*this);
                        Square to(0, 2);
                        Move move(from, to);

                        /* Unset castle flags */
                        result.w_kingside = result.w_queenside = false;

                        /* Move king, rook */
                        result.board[from.get_index()] = Piece();
                        result.board[to.get_index()] = 'K';
                        result.board[Square(0, 3).get_index()] = 'R';
                        result.board[Square(0, 0).get_index()] = Piece();

                        result.en_passant_target = Square();
                        result.color_to_move = colorflip(result.color_to_move);

                        result.occ.flip(from);
                        result.occ.flip(to);
                        result.occ.flip(Square(0, 0));
                        result.occ.flip(Square(0, 3));

                        result.white_king_mask = ((u64) 1 << (to.get_index()));

                        out->push_back(Transition(move, result));
                    }
                }
            }
        }
    } else {
        Square from(7, 4);

        if (b_kingside) {
            u64 no_attack_mask = 0;

            no_attack_mask |= ((u64) 1 << Square(7, 4).get_index());
            no_attack_mask |= ((u64) 1 << Square(7, 5).get_index());
            no_attack_mask |= ((u64) 1 << Square(7, 6).get_index());

            /* Check the rook is in the corner. */
            if (board[Square(7, 7).get_index()].get_uci() == 'r') {
                /* Check the 3 target squares are not in check. */
                if (!(white_attack_mask & no_attack_mask)) {
                    /* Check the squares bwteen the king and rook are empty. */
                    if (!(occ.get_rank(7) & 0x60)) {
                        /* Make the move! */
                        Position result(*this);
                        Square to(7, 6);
                        Move move(from, to);

                        /* Unset castle flags */
                        result.b_kingside = result.b_queenside = false;

                        /* Move king, rook */
                        result.board[from.get_index()] = Piece();
                        result.board[to.get_index()] = 'k';
                        result.board[Square(7, 5).get_index()] = 'r';
                        result.board[Square(7, 7).get_index()] = Piece();

                        result.en_passant_target = Square();
                        result.color_to_move = colorflip(result.color_to_move);

                        result.occ.flip(from);
                        result.occ.flip(to);
                        result.occ.flip(Square(7, 5));
                        result.occ.flip(Square(7, 7));

                        result.black_king_mask = ((u64) 1 << (to.get_index()));

                        out->push_back(Transition(move, result));
                    }
                }
            }
        }

        if (b_queenside) {
            u64 no_attack_mask = 0;

            no_attack_mask |= ((u64) 1 << Square(7, 4).get_index());
            no_attack_mask |= ((u64) 1 << Square(7, 3).get_index());
            no_attack_mask |= ((u64) 1 << Square(7, 2).get_index());

            /* Check the rook is in the corner. */
            if (board[Square(7, 0).get_index()].get_uci() == 'r') {
                /* Check the 3 target squares are not in check. */
                if (!(white_attack_mask & no_attack_mask)) {
                    /* Check the squares bwteen the king and rook are empty. */
                    if (!(occ.get_rank(7) & 0x0E)) {
                        /* Make the move! */
                        Position result(*this);
                        Square to(7, 2);
                        Move move(from, to);

                        /* Unset castle flags */
                        result.b_kingside = result.b_queenside = false;

                        /* Move king, rook */
                        result.board[from.get_index()] = Piece();
                        result.board[to.get_index()] = 'k';
                        result.board[Square(7, 3).get_index()] = 'r';
                        result.board[Square(7, 0).get_index()] = Piece();

                        result.en_passant_target = Square();
                        result.color_to_move = colorflip(result.color_to_move);

                        result.occ.flip(from);
                        result.occ.flip(to);
                        result.occ.flip(Square(7, 0));
                        result.occ.flip(Square(7, 3));

                        result.black_king_mask = ((u64) 1 << (to.get_index()));

                        out->push_back(Transition(move, result));
                    }
                }
            }
        }
    }
}

void Position::get_pseudolegal_rook_moves(Square from, std::list<Position::Transition>* out) {
    /* N */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank() + d, from.get_file());

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }

    /* E */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank(), from.get_file() + d);

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }

    /* S */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank() - d, from.get_file());

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }

    /* W */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank(), from.get_file() - d);

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }
}

void Position::get_pseudolegal_bishop_moves(Square from, std::list<Position::Transition>* out) {
    /* NE */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank() + d, from.get_file() + d);

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }

    /* NW */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank() + d, from.get_file() - d);

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }

    /* SE */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank() - d, from.get_file() + d);

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }

    /* SW */
    for (int d = 1;; ++d) {
        Square dst(from.get_rank() - d, from.get_file() - d);

        if (!dst.is_valid()) break;

        if (board[dst.get_index()].is_valid()) {
            if (board[dst.get_index()].get_color() != color_to_move) {
                out->push_back(make_basic_pseudolegal_move(from, dst));
            }

            break;
        } else {
            out->push_back(make_basic_pseudolegal_move(from, dst));
        }
    }
}

Position::Transition Position::make_basic_pseudolegal_move(Square from, Square to, char promote_type) {
    Piece* pto = board + to.get_index();
    Piece* pfrom = board + from.get_index();

    Position result(*this);
    Move move(from, to, promote_type);

    /* unset en passant target */
    result.en_passant_target = Square();

    /* Check if the EP target needs to be reset */
    if (pfrom->get_type() == 'p') {
        int rdiff = to.get_rank() - from.get_rank();

        if (rdiff == 2 || rdiff == -2) {
            result.en_passant_target = Square(from.get_rank() + rdiff / 2, to.get_file());
        }

        /* Make sure to remove the attacked pawn in an EP capture */
        if (from.get_file() != to.get_file() && !result.board[to.get_index()].is_valid()) {
            Square target(from.get_rank(), to.get_file());
            result.board[target.get_index()] = Piece();
            result.occ.flip(target);
        }
    }

    /* Update board squares */
    result.board[to.get_index()] = result.board[from.get_index()];
    result.board[from.get_index()] = Piece();

    /* Update occupancy */
    result.occ.flip(from);

    if (!pto->is_valid()) {
        result.occ.flip(to);
    }

    /* Update move number if it is black's move */
    if (color_to_move == 'b') {
        ++result.fullmove_number;
    }

    /* Update color to move */
    result.color_to_move = (color_to_move == 'w') ? 'b' : 'w';

    if (promote_type) {
        /* Promote the piece! */
        result.board[to.get_index()].set(color_to_move, promote_type);
    }

    /* Update king mask */
    if (pfrom->get_type() == 'k') {
        if (pfrom->get_color() == 'w') {
            result.white_king_mask = ((u64) 1 << (to.get_index()));
            result.w_kingside = result.w_queenside = false;
        } else {
            result.black_king_mask = ((u64) 1 << (to.get_index()));
            result.b_kingside = result.b_queenside = false;
        }
    }

    /* No castling if rooks are moved. */
    if (from.get_index() == 63) {
        result.b_kingside = false;
    }

    if (from.get_index() == 56) {
        result.b_queenside = false;
    }

    if (from.get_index() == 7) {
        result.b_kingside = false;
    }

    if (from.get_index() == 0) {
        result.w_queenside = false;
    }

    result.set_quiet(!pto->is_valid());

    return Position::Transition(move, result);
}

bool Position::get_color_in_check(char col) {
    if (col == 'w') return white_in_check;
    return black_in_check;
}

float Position::get_eval() {
    float phase = eval_get_phase();
    float eval_result = 0.0f;

    /* Like development in the opening. */
    eval_result += (1.0f - phase) * (eval_development('w') - eval_development('b'));

    /* Like material throughout the game. */
    eval_result += (eval_material('w') - eval_material('b'));

    /* Like center control from the opening to the middlegame. */
    if (phase <= 0.5) {
        eval_result += ((0.5f - phase) * 2.0f) * (eval_center('w') - eval_development('b'));
    }

    return eval_result;
}

float Position::eval_get_phase() {
    /* Count total non-pawn material. */
    float npm = 0.0f;

    for (int i = 0; i < 64; ++i) {
        switch (board[i].get_type()) {
        case 'k':
        case 'p':
            break;
        default:
            npm += eval_get_piece_value(board[i].get_type());
        }
    }

    return 1.0f - (npm / NPM_TOTAL);
}

float Position::eval_center(char c) {
    static constexpr u64 CENTER_MASK = ((1ULL << 35) | (1ULL << 36) | (1ULL << 27) | (1ULL << 28));

    u64 mask = (c == 'w') ? white_attack_mask : black_attack_mask;

    return __builtin_popcountll(CENTER_MASK & mask) / 4.0f;
}

float Position::eval_development(char c) {
    int r = (c == 'w') ? 2 : 4;
    float total = 0.0f;

    for (int a = 0; a < 2; ++a) {
        for (int f = 0; f < 8; ++f) {
            switch (board[(r+a)*8+f].get_type()) {
            case 'b':
            case 'n':
            case 'r':
            case 'q':
                total += 1.0f;
                break;
            }
        }
    }

    return total;
}

float Position::eval_get_piece_value(char p) {
    switch (p) {
    case 'p':
        return MAT_PAWN;
    case 'b':
        return MAT_BISHOP;
    case 'n':
        return MAT_KNIGHT;
    case 'r':
        return MAT_ROOK;
    case 'q':
        return MAT_QUEEN;
    default:
        return 0.0f;
    }
}

float Position::eval_material(char c) {
    float total = 0.0f;

    for (int i = 0; i < 64; ++i) {
        if (board[i].get_color() == c) {
            total += eval_get_piece_value(board[i].get_type());
        }
    }

    return total;
}

char Position::colorflip(char c) {
    return (c == 'w') ? 'b' : 'w';
}

Position::Transition::Transition(Move _move, Position _result, bool _check, bool _mate) : move(_move) {
    check = _check;
    mate = _mate;
    result = new Position(_result);
}

Position::Transition::~Transition() {
    delete result;
}

Position::Transition::Transition(const Position::Transition& b) : move(b.move) {
    check = b.check;
    mate = b.mate;

    result = new Position(*(b.result));
}

Position* Position::Transition::get_result() {
    return result;
}

Move* Position::Transition::get_move() {
    return &move;
}

bool Position::Transition::get_check() {
    return check;
}

bool Position::Transition::get_mate() {
    return mate;
}

void Position::Transition::set_check(bool _check) {
    check = _check;
}

void Position::Transition::set_mate(bool _mate) {
    mate = _mate;
}

std::string Position::Transition::to_string() {
    std::string out = move.to_string();

    if (mate) {
        out += '#';
    } else if (check) {
        out += '+';
    }

    return out;
}

Position::Transition::operator std::string() {
    return to_string();
}

char Position::get_color_to_move() {
    return color_to_move;
}

void Position::compute_attack_masks() {
    /*nc_debug("Computing attack masks for %s", get_fen().c_str());

    nc_debug("white king mask:");
    std::cerr << bitboard::to_string(white_king_mask) << "\n";
    nc_debug("black king mask:");
    std::cerr << bitboard::to_string(black_king_mask) << "\n";*/

    white_attack_mask = 0;
    black_attack_mask = 0;

    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            Square dst(r, f);
            Piece* p = board + dst.get_index();

            if (!p->is_valid()) continue;

            u64* dst_mask = (p->get_color() == 'w') ? &white_attack_mask : &black_attack_mask;

            switch (p->get_type()) {
            case 'k':
                *dst_mask |= lookup::king_attack(dst);
                break;
            case 'q':
                *dst_mask |= lookup::queen_attack(dst, &occ);
                break;
            case 'b':
                *dst_mask |= lookup::bishop_attack(dst, &occ);
                break;
            case 'n':
                *dst_mask |= lookup::knight_attack(dst);
                break;
            case 'r':
                *dst_mask |= lookup::rook_attack(dst, &occ);
                break;
            case 'p':
                if (p->get_color() == 'w') {
                    *dst_mask |= lookup::white_pawn_attack(dst);
                } else {
                    *dst_mask |= lookup::black_pawn_attack(dst);
                }
                break;
            }
        }
    }

    /*nc_debug("white attack mask:");
    std::cerr << bitboard::to_string(white_king_mask) << "\n";
    nc_debug("black attack mask:");
    std::cerr << bitboard::to_string(black_attack_mask) << "\n";*/

    white_in_check = (black_attack_mask & white_king_mask);
    black_in_check = (white_attack_mask & black_king_mask);
}

bool Position::is_quiet() {
    return (!white_in_check && !black_in_check) && quiet;
}

void Position::set_quiet(bool q) {
    quiet = q;
}
