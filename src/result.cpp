#include "result.h"

#include <cstring>
#include <stdexcept>

using namespace nc2;

Result::Result(u64 key, Evaluation score, std::list<Move> pv) : key(key), score(score), pv(pv), depth(0) {}

u64 Result::get_key() {
    return key;
}

Move Result::get_bestmove() {
    if (!pv.size()) return Move();
    return pv.front();
}

Evaluation Result::get_score() {
    return score;
}

std::list<Move> Result::get_pv() {
    return pv;
}

std::string Result::get_pv_string() {
    std::string out;
    unsigned count = pv.size();

    for (auto m : pv) {
        out += m.to_string();
        if (++count == pv.size()) break;
        out += ' ';
    }

    return out;
}

int Result::get_depth() {
    return depth;
}

void Result::insert_move(u64 new_key, Move m, int depth_inc) {
    key = new_key;
    pv.insert(pv.begin(), m);
    depth += depth_inc;

    if (score.get_forced_mate()) {
        if (score.get_mate_in() > 0) {
            score.set_mate_in(depth / 2 + 1);
        } else if (score.get_mate_in() < 0) {
            score.set_mate_in(-depth / 2 - 1);
        } else {
            if (score.get_eval() > 0.0f) {
                score.set_mate_in(1);
            } else if (score.get_eval() < 0.0f) {
                score.set_mate_in(-1);
            } else {
                throw std::runtime_error("Unexpected insert_move on #0...");
            }
        }
    }
}

void Result::update(Result& from) {
    this->score = from.score;
    this->pv = from.pv;
    this->depth = from.depth;
}
