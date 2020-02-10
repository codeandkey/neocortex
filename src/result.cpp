#include "result.h"

#include <cstring>

using namespace nc2;

search::Result::Result(Position pos, Evaluation score, std::list<Move> pv) : pos(pos), score(score), pv(pv), depth(0) {}

Move search::Result::get_bestmove() {
    if (!pv.size()) return Move();
    return pv.front();
}

Evaluation search::Result::get_score() {
    return score;
}

float search::Result::get_current() {
    return pos.get_eval_heuristic();
}

Position* search::Result::get_position() {
    return &pos;
}

std::list<Move> search::Result::get_pv() {
    return pv;
}

std::string search::Result::get_pv_string() {
    std::string out;
    unsigned count = pv.size();

    for (auto m : pv) {
        out += m.to_string();
        if (++count == pv.size()) break;
        out += ' ';
    }

    return out;
}

int search::Result::get_depth() {
    return depth;
}

bool search::Result::check_position(Position* rhs) {
    return !memcmp(rhs->get_board(), pos.get_board(), 64);
}

void search::Result::insert_move(Position before_pos, Move m, int depth_inc) {
    pv.insert(pv.begin(), m);

    if (score.get_forced_mate()) {
        if (!score.get_mate_in()) {
            if (before_pos.get_color_to_move() == piece::Color::WHITE) {
                score.set_mate_in(1);
            } else {
                score.set_mate_in(-1);
            }
        } else if (score.get_mate_in() > 0 && before_pos.get_color_to_move() == piece::Color::WHITE) {
            score.set_mate_in(score.get_mate_in() + 1);
        } else if (score.get_mate_in() < 0 && before_pos.get_color_to_move() == piece::Color::BLACK) {
            score.set_mate_in(score.get_mate_in() - 1);
        }
    }

    pos = before_pos;
    depth += depth_inc;
}

void search::Result::update(search::Result& from) {
    /* Here, we decide the transposition table replacement strategy.
     * For now, we will always replace colliding nodes. */

    this->score = from.score;
    this->pv = from.pv;
    this->depth = from.depth;
}
