#include "searcher_st.h"
#include "ttable.h"
#include "eval.h"

#include <algorithm>
#include <chrono>

using namespace nc2;

SearcherST::SearcherST(std::ostream& uci_out) : uci_out(uci_out) {}

void SearcherST::set_position(Position p) {
    root = p;
}

void SearcherST::go(int wtime, int btime) {
    int movetime = (root.get_color_to_move() == piece::Color::WHITE) ? wtime : btime;
    int search_depth = DEPTH;

    search::Result last_result;

    if (movetime > 0) {
        if (movetime < 3500) {
            search_depth = 1;
        } else if (movetime < 7000) {
            search_depth = 2;
        } else if (movetime < 12000) {
            search_depth = 3;
        } else if (movetime < 20000) {
            search_depth = 4;
        } else if (movetime < 120000) {
            search_depth = 5;
        } else if (movetime < 300000) {
            search_depth = 6;
        } else {
            search_depth = 7;
        }
    } else {
        search_depth = 7;
    }

    int i = 0;
    for (; i <= search_depth; ++i) {
        nodes = 0;
        thits = 0;

        auto cur_time = std::chrono::system_clock::now();

        last_result = alpha_beta(&root, i, Evaluation(0, true, -1), Evaluation(0, true, 1));

        auto post_time = std::chrono::system_clock::now();
        int ms = std::chrono::duration_cast<std::chrono::milliseconds>(post_time - cur_time).count();
        int nps = (float) nodes * 1000.0f / (ms + 1);

        std::cerr << "info depth " << i << " nodes " << nodes << " thits " << thits << " nps " << nps << " time " << ms << " score " << last_result.get_score().to_string() << " pv " << last_result.get_pv_string() << "\n";
        uci_out << "info depth " << i << " nodes " << nodes << " thits " << thits << " nps " << nps << " time " << ms << " score " << last_result.get_score().to_uci_string() << " pv " << last_result.get_pv_string() << "\n";
    }

    std::cerr << "Search stopped at depth " << (i - 1) << ", evaluation " << last_result.get_score().to_string() << " move " << last_result.get_bestmove().to_string() << "\n";

    uci_out << "bestmove " << last_result.get_bestmove().to_string() << "\n";
}

search::Result SearcherST::alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta) {
    ++nodes;

    if (!d) {
        if (p->is_quiet()) {
            return search::Result(*p, Evaluation(p->get_eval_heuristic()));
        } else {
            return quiescence(p, QDEPTH, alpha, beta);
        }
    }

    /* Load any potential thits. */
    search::Result* current_hit = ttable::lookup(p);

    if (current_hit) {
        if (current_hit->get_depth() >= d) {
            return *current_hit;
        }
    }

    /* No thit, grab legal next moves from position. */
    std::vector<Position::Transition> legal_moves = p->gen_legal_moves();

    if (!legal_moves.size()) {
        if (!p->get_color_in_check(p->get_color_to_move())) {
            return search::Result(*p, Evaluation(0, false, 0), std::list<Move>());
        } else {
            return search::Result(*p, Evaluation(0, true, 0), std::list<Move>());
        }
    }

    if (p->get_color_to_move() == piece::Color::WHITE) {
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval_heuristic() > b.second.get_eval_heuristic();
        });

        /* Maximize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, -1);
        best_lines.push_back(Edge(legal_moves[0].first, search::Result(*p, current_best_score, std::list<Move>{legal_moves[0].first})));

        for (auto m : legal_moves) {
            search::Result inner = alpha_beta(&m.second, d - 1, alpha, beta);

            if (inner.get_score().get_forced_mate() && inner.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));
                break;
            } else if (inner.get_score() == current_best_score) {
                best_lines.push_back(Edge(m.first, inner));
            } else if (inner.get_score() > current_best_score) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));

                current_best_score = inner.get_score();

                if (current_best_score > alpha) {
                    alpha = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_current() > b.second.get_current();
        });

        /* Push the new move to the PV and return the new result. */
        search::Result best_line = best_lines.front().second;

        best_line.insert_move(*p, best_lines.front().first);

        /* Position wasn't a table hit, so store it in the ttable. */
        ttable::store(p, best_line);

        return best_line;
    } else {
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval_heuristic() < b.second.get_eval_heuristic();
        });

        /* Minimize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, 1);
        best_lines.push_back(Edge(legal_moves[0].first, search::Result(*p, current_best_score, std::list<Move>{legal_moves[0].first})));

        for (auto m : legal_moves) {
            search::Result inner = alpha_beta(&m.second, d - 1, alpha, beta);

            if (inner.get_score().get_forced_mate() && inner.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));
                break;
            } else if (inner.get_score() == current_best_score) {
                best_lines.push_back(Edge(m.first, inner));
            } else if (inner.get_score() < current_best_score) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));

                current_best_score = inner.get_score();

                if (current_best_score < beta) {
                    beta = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_current() < b.second.get_current();
        });

        /* Push the new move to the PV and return the new result. */
        search::Result best_line = best_lines.front().second;

        best_line.insert_move(*p, best_lines.front().first);

        /* Position wasn't a table hit, so store it in the ttable. */
        ttable::store(p, best_line);

        return best_line;
    }
}

search::Result SearcherST::quiescence(Position* p, int d, Evaluation alpha, Evaluation beta) {
    ++nodes;

    if (!d || p->is_quiet()) {
        return search::Result(*p, p->get_eval_heuristic());
    }

    std::vector<Position::Transition> legal_moves = p->gen_legal_moves();

    if (!legal_moves.size()) {
        if (!p->get_color_in_check(p->get_color_to_move())) {
            return search::Result(*p, Evaluation(0, false, 0));
        } else {
            return search::Result(*p, Evaluation(0, true, 0));
        }
    }

    if (p->get_color_to_move() == piece::Color::WHITE) {
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval_heuristic() > b.second.get_eval_heuristic();
        });

        /* Maximize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, -1);
        best_lines.push_back(Edge(legal_moves[0].first, search::Result(*p, current_best_score, std::list<Move>{legal_moves[0].first})));

        for (auto m : legal_moves) {
            search::Result inner = quiescence(&m.second, d - 1, alpha, beta);

            if (inner.get_score().get_forced_mate() && inner.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));
                break;
            } else if (inner.get_score() == current_best_score) {
                best_lines.push_back(Edge(m.first, inner));
            } else if (inner.get_score() > current_best_score) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));

                current_best_score = inner.get_score();

                if (current_best_score > alpha) {
                    alpha = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_current() > b.second.get_current();
        });

        /* Push the new move to the PV and return the new result. */
        search::Result best_line = best_lines.front().second;

        /* When inserting quiescence moves, don't consider them a searched depth. */
        best_line.insert_move(*p, best_lines.front().first, 0);

        return best_line;
    } else {
        std::sort(legal_moves.begin(), legal_moves.end(), [=](Position::Transition& a, Position::Transition& b) {
            return a.second.get_eval_heuristic() < b.second.get_eval_heuristic();
        });

        /* Minimize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, 1);
        best_lines.push_back(Edge(legal_moves[0].first, search::Result(*p, current_best_score, std::list<Move>{legal_moves[0].first})));

        for (auto m : legal_moves) {
            search::Result inner = quiescence(&m.second, d - 1, alpha, beta);

            if (inner.get_score().get_forced_mate() && inner.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));
                break;
            } else if (inner.get_score() == current_best_score) {
                best_lines.push_back(Edge(m.first, inner));
            } else if (inner.get_score() < current_best_score) {
                best_lines.clear();
                best_lines.push_back(Edge(m.first, inner));

                current_best_score = inner.get_score();

                if (current_best_score < beta) {
                    beta = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_current() < b.second.get_current();
        });

        /* Push the new move to the PV and return the new result. */
        search::Result best_line = best_lines.front().second;

        /* When inserting quiescence moves, don't consider them a searched depth. */
        best_line.insert_move(*p, best_lines.front().first, 0);

        return best_line;
    }
}
