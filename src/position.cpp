#include "position.h"
#include "lookup.h"

#include <stdexcept>

using namespace nc;

Position::Position() {
    /* Standard game init */
    white_in_check = black_in_check = false;
    w_kingside = w_queenside = b_kingside = b_queenside = true;
    color_to_move = 'w';

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
    throw std::runtime_error("FEN parsing not implemented! FIXME");
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

        out += '/';
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

    /* Then, prune out those which leave the moving color in check. */
    auto psl_iter = out.begin();

    while (psl_iter != out.end()) {
        Position* result = (*psl_iter).get_result();

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

                            out.push_back(make_basic_pseudolegal_move(Square(r, f), capture_dst));
                        }
                    }
                    break;
                }
            }
        }
    }

    return out;
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

    return Position::Transition(move, result);
}

bool Position::get_color_in_check(char col) {
    /* Make a bitmask for the king. */
    u64 king_mask = 0;

    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            Piece* p = board + Square(r, f).get_index();

            if (p->get_type() == 'k' && p->get_color() == col) {
                king_mask = ((u64) 1 << (r * 8 + f));
                break;
            }
        }
    }

    /* Construct an attack mask and see if it intersects with the king mask. */
    u64 attack_mask = 0;

    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            Square dst(r, f);
            Piece* p = board + dst.get_index();

            if (p->is_valid() && p->get_color() != col) {
                switch (p->get_type()) {
                case 'k':
                    attack_mask |= lookup::king_attack(dst);
                    break;
                case 'q':
                    attack_mask |= lookup::queen_attack(dst, &occ);
                    break;
                case 'b':
                    attack_mask |= lookup::bishop_attack(dst, &occ);
                    break;
                case 'n':
                    attack_mask |= lookup::knight_attack(dst);
                    break;
                case 'r':
                    attack_mask |= lookup::rook_attack(dst, &occ);
                    break;
                case 'p':
                    if (col == 'w') {
                        attack_mask |= lookup::black_pawn_attack(dst);
                    } else {
                        attack_mask |= lookup::white_pawn_attack(dst);
                    }
                    break;
                }
            }

            /* Check for mask intersection. */
            if (attack_mask & king_mask) {
                return true;
            }
        }
    }

    return false;
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
