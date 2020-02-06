#include "searcher_st.h"
#include "ttable.h"

#include <algorithm>
#include <chrono>

using namespace nc2;

SearcherST::SearcherST(std::ostream& uci_out) : uci_out(uci_out) {}

void SearcherST::set_position(Position p) {
    root = p;
}

void SearcherST::go() {
    std::vector<Position::Transition> moves = root.gen_legal_moves();

    nodes = 0;
    thits = 0;

    auto cur_time = std::chrono::system_clock::now();
    Move bestmove;
    Evaluation current_eval = alpha_beta(&root, DEPTH, Evaluation(0, true, -1), Evaluation(0, true, 1), &bestmove);

    auto post_time = std::chrono::system_clock::now();

    float seconds = std::chrono::duration_cast<std::chrono::milliseconds>(post_time - cur_time).count() / 1000.0f;

    int nps = (float) nodes / (float) seconds;

    std::cerr << "Search completed at depth " << DEPTH << ", evaluation " << current_eval.to_string() << " bestmove " << bestmove.to_string() << " nodes " << nodes << " nps " << nps << " thits " << thits << " time " << seconds << "\n";
    uci_out << "bestmove " << bestmove.to_string() << "\n";
}

Evaluation SearcherST::alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta, Move* bestmove_out) {
    ++nodes;

    /* Check the ttable cache. */
    /* Don't try a table lookup if we need to report a bestmove. */

    if (!bestmove_out) {
        Evaluation ttable_hit(0, false, 0);

        if (ttable::lookup(p, &ttable_hit, d)) {
            ++thits;
            return ttable_hit;
        }
    }

    if (!d) {
        if (p->is_quiet()) {
            return Evaluation(p->get_eval());
        } else {
            return quiescence(p, QDEPTH, alpha, beta);
        }
    }

    std::vector<Position::Transition> legal_moves = p->gen_legal_moves();

    if (!legal_moves.size()) {
        if (bestmove_out) {
            throw std::runtime_error("Bestmove requested, but no legal moves..");
        }

        if (!p->get_color_in_check(p->get_color_to_move())) {
            return Evaluation(0, false, 0);
        } else {
            return Evaluation(0, true, 0);
        }
    }

    if (bestmove_out) *bestmove_out = legal_moves[0].first;

    if (p->get_color_to_move() == piece::Color::WHITE) {
        /* Sort legal moves by best heuristic first. This helps speed up AB pruning. */
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval() > b.second.get_eval();
        });

        /* Maximize evaluation. */
        std::vector<Position::Transition> best_moves;
        best_moves.push_back(legal_moves[0]);
        Evaluation best_eval(0, true, -1);

        for (auto m : legal_moves) {
            Evaluation inner = alpha_beta(&m.second, d - 1, alpha, beta, nullptr);

            if (d == DEPTH) {
                std::cerr << "evaluated top-level move " << m.first.to_string() << " : search " << inner.to_string() << " immediate " << m.second.get_eval() << "\n";
            }

            if (inner.get_forced_mate() && !inner.get_mate_in()) {
                /* Move is mate in 1! */
                if (bestmove_out) *bestmove_out = m.first;
                return Evaluation(0, true, 1);
            }

            if (inner == best_eval) {
                best_moves.push_back(m);
            } else if (inner > best_eval) {
                best_moves.clear();
                best_moves.push_back(m);
                best_eval = inner;

                if (best_eval > alpha) {
                    alpha = best_eval;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best move from all the best moves. */
        if (bestmove_out) {
            std::sort(best_moves.begin(), best_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
                return a.second.get_eval() > b.second.get_eval();
            });

            *bestmove_out = best_moves[0].first;
        }

        /* Position wasn't a table hit, so store it in the ttable. */
        ttable::store(p, best_eval, d);

        return best_eval;
    } else {
        /* Sort legal moves by best heuristic first. This helps speed up AB pruning. */
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval() < b.second.get_eval();
        });

        /* Minimize evaluation. */
        std::vector<Position::Transition> best_moves;
        best_moves.push_back(legal_moves[0]);
        Evaluation best_eval(0, true, 1);

        for (auto m : legal_moves) {
            Evaluation inner = alpha_beta(&m.second, d - 1, alpha, beta, nullptr);

            if (d == DEPTH) {
                std::cerr << "evaluated top-level move " << m.first.to_string() << " : search " << inner.to_string() << " immediate " << m.second.get_eval() << "\n";
            }

            if (inner.get_forced_mate() && !inner.get_mate_in()) {
                /* Move is mate in 1! */
                if (bestmove_out) *bestmove_out = m.first;
                return Evaluation(0, true, -1);
            }

            if (inner == best_eval) {
                best_moves.push_back(m);
            } else if (inner < best_eval) {
                best_moves.clear();
                best_moves.push_back(m);
                best_eval = inner;

                if (best_eval < beta) {
                    beta = best_eval;

                    if (alpha > beta) break;
                }
            }
        }

        if (bestmove_out) {
            /* Choose the best move from all the best moves. */
            std::sort(best_moves.begin(), best_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
                return a.second.get_eval() < b.second.get_eval();
            });

            *bestmove_out = best_moves[0].first;
        }

        /* Position wasn't a table hit, so store it in the ttable. */
        ttable::store(p, best_eval, d);

        return best_eval;
    }
}

Evaluation SearcherST::quiescence(Position* p, int d, Evaluation alpha, Evaluation beta) {
    ++nodes;

    if (!d || p->is_quiet()) {
        return Evaluation(p->get_eval());
    }

    std::vector<Position::Transition> legal_moves = p->gen_legal_moves();

    if (!legal_moves.size()) {
        if (!p->get_color_in_check(p->get_color_to_move())) {
            return Evaluation(0, false, 0);
        } else {
            return Evaluation(0, true, 0);
        }
    }

    if (p->get_color_to_move() == piece::Color::WHITE) {
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval() > b.second.get_eval();
        });

        /* Maximize evaluation. */
        Evaluation out(0, true, -1);

        for (auto m : legal_moves) {
            Evaluation inner = quiescence(&m.second, d - 1, alpha, beta);

            if (inner.get_forced_mate() && !inner.get_mate_in()) {
                /* Move is mate in 1! */
                return Evaluation(0, true, 1);
            }

            if (inner > out) {
                out = inner;
            }

            if (out > alpha) {
                alpha = out;
            }

            if (alpha > beta) {
                break;
            }
        }

        return out;
    } else {
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval() < b.second.get_eval();
        });

        /* Minimize evaluation. */
        Evaluation out(0, true, 1);

        for (auto m : legal_moves) {
            Evaluation inner = quiescence(&m.second, d - 1, alpha, beta);

            if (inner.get_forced_mate() && !inner.get_mate_in()) {
                /* Move is mate in 1! */
                return Evaluation(0, true, 1);
            }

            if (inner < out) {
                out = inner;
            }

            if (out < beta) {
                beta = out;
            }

            if (alpha > beta) {
                break;
            }
        }

        return out;
    }
}
