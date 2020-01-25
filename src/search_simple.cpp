#include "search_simple.h"
#include "log.h"

#include <vector>

using namespace nc;

SearchSimple::SearchSimple(std::ostream& uci_out) : Search(uci_out), depth(4), quiescence_depth(3) {
    search_running.store(false);
}

void SearchSimple::set_position(Position p) {
    root_position = p;
}

void SearchSimple::go() {
    if (search_worker.joinable()) {
        nc_debug("Search already running, stopping..");
        stop();
    }

    nc_debug("Starting search worker thread..");
    search_worker = std::thread(&SearchSimple::worker_func, this);
}

void SearchSimple::stop() {
    /* Because the search is a recursive alpha-beta there are no best moves we can give before
     * the search is completed. So we can just ignore UCI stops. */

    nc_debug("Joining worker..");
    search_worker.join();
}

void SearchSimple::worker_func() {
    /* Start alpha-beta search. */
    nc_debug("In search worker thread. Starting alpha-beta.. ");

    Move bestmove;
    Evaluation eval = alpha_beta(&root_position, depth, Evaluation(0, true, -1), Evaluation(0, true, 1), &bestmove);

    nc_debug("%s: evaluation %s, bestmove %s", root_position.get_fen().c_str(), eval.to_string().c_str(), bestmove.to_string().c_str());

    write_bestmove(bestmove);
}

Evaluation SearchSimple::alpha_beta(Position* p, int d, Evaluation alpha, Evaluation beta, Move* bestmove) {
    if (!d) {
        if (p->is_quiet()) {
            return Evaluation(p->get_eval());
        } else {
            return quiescence_search(p, quiescence_depth, alpha, beta);
        }
    }

    std::list<Position::Transition> legal_moves = p->get_legal_moves();

    if (bestmove) {
        if (!legal_moves.size()) {
            throw std::runtime_error("Best move requested, but no legal moves..");
        }

        *bestmove = *(legal_moves.front().get_move());
    }

    if (!legal_moves.size() && !(p->get_color_in_check(p->get_color_to_move()))) {
        return Evaluation(0, false, 0); /* stalemate endgame */
    }

    if (p->get_color_to_move() == 'w') {
        /* Maximize evaluation. */
        Evaluation out(0, true, -1);

        for (auto m : legal_moves) {
            if (m.get_mate()) {
                if (bestmove) *bestmove = *(m.get_move());
                return Evaluation(0, true, 1);
            }

            Evaluation inner = alpha_beta(m.get_result(), d - 1, alpha, beta, nullptr);

            if (inner > out) {
                out = inner;
                if (bestmove) *bestmove = *(m.get_move());
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
        /* Minimize evaluation. */
        Evaluation out(0, true, 1);

        for (auto m : legal_moves) {
            if (m.get_mate()) {
                if (bestmove) *bestmove = *(m.get_move());
                return Evaluation(0, true, -1);
            }

            Evaluation inner = alpha_beta(m.get_result(), d - 1, alpha, beta, nullptr);

            if (inner < out) {
                out = inner;
                if (bestmove) *bestmove = *(m.get_move());
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

Evaluation SearchSimple::quiescence_search(Position* p, int d, Evaluation alpha, Evaluation beta) {
    if (!d || p->is_quiet()) {
        return Evaluation(p->get_eval());
    }

    std::list<Position::Transition> legal_moves = p->get_legal_moves();

    if (p->get_color_to_move() == 'w') {
        /* Maximize evaluation. */
        Evaluation out(0, true, -1);

        for (auto m : legal_moves) {
            if (m.get_mate()) {
                return Evaluation(0, true, 1);
            }

            Evaluation inner = quiescence_search(m.get_result(), d - 1, alpha, beta);

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
        /* Minimize evaluation. */
        Evaluation out(0, true, 1);

        for (auto m : legal_moves) {
            if (m.get_mate()) {
                return Evaluation(0, true, -1);
            }

            Evaluation inner = quiescence_search(m.get_result(), d - 1, alpha, beta);

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
