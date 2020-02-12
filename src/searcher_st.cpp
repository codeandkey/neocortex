#include "searcher_st.h"
#include "ttable.h"
#include "eval.h"

#include <algorithm>
#include <chrono>

using namespace nc2;

SearcherST::SearcherST(std::ostream& uci_out) : uci_out(uci_out) {}

void SearcherST::go(int wtime, int btime) {
    int movetime = (root.get_color_to_move() == piece::Color::WHITE) ? wtime : btime;
    int search_depth;

    Result last_result;

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
        uci_out << "info depth " << i << " nodes " << nodes << " nps " << nps << " time " << ms << " score " << last_result.get_score().to_uci_string() << " pv " << last_result.get_pv_string() << "\n";
    }

    std::cerr << "Search stopped at depth " << (i - 1) << ", evaluation " << last_result.get_score().to_string() << " move " << last_result.get_bestmove().to_string() << "\n";

    uci_out << "bestmove " << last_result.get_bestmove().to_string() << "\n";
}

void SearcherST::set_position(Position p) {
    root = p;
}

Result SearcherST::run_search(Position* start, int depth) {
    Result output;

    for (int i = 1; i <= depth; ++i) {
        output = alpha_beta(start, i, Evaluation(0, true, -1), Evaluation(0, true, 1));
    }

    return output;
}

Result SearcherST::alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta) {
    ++nodes;

    if (!d) {
        if (p->is_quiet()) {
            return p->get_best_line(&thits);
        } else {
            return quiescence(p, alpha, beta);
        }
    }

    /* Check if the current position is a good thit. */
    if (p->get_best_line(&thits).get_depth() >= d) {
        return p->get_best_line(&thits);
    }

    /* No thit, grab legal next moves from position. */
    std::vector<Position::Transition> legal_moves = p->gen_legal_moves();

    if (!legal_moves.size()) {
        if (!p->get_color_in_check(p->get_color_to_move())) {
            return Result(p->get_ttable_key(), Evaluation(0, false, 0));
        } else {
            return Result(p->get_ttable_key(), Evaluation(p->get_color_to_move()));
        }
    }

    /* Make into list of edges and try early lookups. */
    std::list<Edge> next_edges;

    for (auto m : legal_moves) {
        next_edges.push_back(Edge(m, m.second.get_best_line(&thits)));
    }

    if (p->get_color_to_move() == piece::Color::WHITE) {
        /* Perform move ordering */
        next_edges.sort([&](Edge& a, Edge& b) {
            return a.second.get_score() > b.second.get_score();
        });

        /* Maximize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, -1);

        for (auto e : next_edges) {
            if (e.second.get_depth() < d - 1) {
                e.second = alpha_beta(&e.first.second, d - 1, alpha, beta);
            }

            if (e.second.get_score().get_forced_mate() && e.second.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(e);
                break;
            } else if (e.second.get_score() == current_best_score) {
                best_lines.push_back(e);
            } else if (e.second.get_score() > current_best_score) {
                best_lines.clear();
                best_lines.push_back(e);

                current_best_score = e.second.get_score();

                if (current_best_score > alpha) {
                    alpha = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_score() > b.second.get_score();
        });

        /* Push the new move to the PV and return the new result. */
        Result best_line = best_lines.front().second;

        best_line.insert_move(p->get_ttable_key(), best_lines.front().first.first);

        p->store_best_line(best_line);

        return best_line;
    } else {
        /* Perform move ordering */
        next_edges.sort([&](Edge& a, Edge& b) {
            return a.second.get_score() < b.second.get_score();
        });

        /* Maximize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, 1);

        for (auto e : next_edges) {
            if (e.second.get_depth() < d - 1) {
                e.second = alpha_beta(&e.first.second, d - 1, alpha, beta);
            }

            if (e.second.get_score().get_forced_mate() && e.second.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(e);
                break;
            } else if (e.second.get_score() == current_best_score) {
                best_lines.push_back(e);
            } else if (e.second.get_score() < current_best_score) {
                best_lines.clear();
                best_lines.push_back(e);

                current_best_score = e.second.get_score();

                if (current_best_score < beta) {
                    beta = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_score() < b.second.get_score();
        });

        /* Push the new move to the PV and return the new result. */
        Result best_line = best_lines.front().second;

        best_line.insert_move(p->get_ttable_key(), best_lines.front().first.first);

        /* Position wasn't a table hit, so store it in the ttable. */
        p->store_best_line(best_line);

        return best_line;
    }
}

Result SearcherST::quiescence(Position* p, Evaluation alpha, Evaluation beta) {
    ++nodes;

    if (p->is_quiet()) {
        return p->get_best_line(&thits);
    }

    std::vector<Position::Transition> legal_moves = p->gen_legal_moves();

    if (!legal_moves.size()) {
        if (!p->get_color_in_check(p->get_color_to_move())) {
            return Result(p->get_ttable_key(), Evaluation(0, false, 0));
        } else {
            return Result(p->get_ttable_key(), Evaluation(p->get_color_to_move()));
        }
    }

    /* Make into list of edges and try early lookups. */
    std::list<Edge> next_edges;

    for (auto m : legal_moves) {
        next_edges.push_back(Edge(m, m.second.get_best_line(&thits)));
    }

    if (p->get_color_to_move() == piece::Color::WHITE) {
        next_edges.sort([&](Edge& lhs, Edge& rhs) {
            return lhs.second.get_score() > rhs.second.get_score();
        });

        /* Maximize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, -1);

        for (auto e : next_edges) {
            Result inner = quiescence(&e.first.second, alpha, beta);

            if (inner.get_score().get_forced_mate() && inner.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(e);
                break;
            } else if (inner.get_score() == current_best_score) {
                best_lines.push_back(e);
            } else if (inner.get_score() > current_best_score) {
                best_lines.clear();
                best_lines.push_back(e);

                current_best_score = inner.get_score();

                if (current_best_score > alpha) {
                    alpha = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_score() > b.second.get_score();
        });

        /* Push the new move to the PV and return the new result. */
        Result best_line = best_lines.front().second;

        /* When inserting quiescence moves, don't consider them a searched depth. */
        best_line.insert_move(p->get_ttable_key(), best_lines.front().first.first, 0);

        return best_line;
    } else {
        next_edges.sort([&](Edge& lhs, Edge& rhs) {
            return lhs.second.get_score() < rhs.second.get_score();
        });

        /* Minimize evaluation. */
        std::list<Edge> best_lines;
        Evaluation current_best_score(0, true, 1);

        for (auto e : next_edges) {
            Result inner = quiescence(&e.first.second, alpha, beta);

            if (inner.get_score().get_forced_mate() && inner.get_score().get_mate_in() == 0) {
                best_lines.clear();
                best_lines.push_back(e);
                break;
            } else if (inner.get_score() == current_best_score) {
                best_lines.push_back(e);
            } else if (inner.get_score() < current_best_score) {
                best_lines.clear();
                best_lines.push_back(e);

                current_best_score = inner.get_score();

                if (current_best_score < beta) {
                    beta = current_best_score;

                    if (alpha > beta) break;
                }
            }
        }

        /* Choose the best line always. */
        best_lines.sort([&](Edge& a, Edge& b) {
            return a.second.get_score() < b.second.get_score();
        });

        /* Push the new move to the PV and return the new result. */
        Result best_line = best_lines.front().second;

        /* When inserting quiescence moves, don't consider them a searched depth. */
        best_line.insert_move(p->get_ttable_key(), best_lines.front().first.first, 0);

        return best_line;
    }
}
