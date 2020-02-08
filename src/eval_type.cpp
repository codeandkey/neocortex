#include "eval_type.h"

#include <stdexcept>

using namespace nc2;

Evaluation::Evaluation(float eval, bool has_mate, int mate_in) {
    this->eval = eval;
    this->has_mate = has_mate;
    this->mate_in = mate_in;

    if (this->has_mate) {
        this->eval = 0.0f;
    }
}

float Evaluation::get_eval() {
    return eval;
}

bool Evaluation::get_forced_mate() {
    return has_mate;
}

int Evaluation::get_mate_in() {
    return mate_in;
}

void Evaluation::set_mate_in(int m) {
    mate_in = m;
}

std::string Evaluation::to_string() {
    std::string out;

    if (has_mate) {
        out += "#";
        out += std::to_string(mate_in);
    } else {
        out += std::to_string(eval);
    }

    return out;
}

std::string Evaluation::to_uci_string() {
    std::string out;

    if (has_mate) {
        out += "mate ";
        out += std::to_string(mate_in);
    } else {
        out += "cp ";
        out += std::to_string((int) (eval * 100.0f));
    }

    return out;
}

Evaluation::operator std::string() {
    return to_string();
}

int Evaluation::compare(const Evaluation& rhs) const {
    /*
     * -1 : RHS better for black
     *  0 : draw
     *  1 : RHS better for white
     */

    if (has_mate) {
        if (rhs.has_mate) {
            /* Both evals have mate. */
            /* If both are for white, the lower one is better for black. */
            /* If both are for black, the lower one is better for white. */

            /* This is mate for white. RHS will be better for black unless it is somehow a better mate for white. */
            if (mate_in > 0) {
                if (rhs.mate_in >= 0 && rhs.mate_in < mate_in) return 1;
                if (rhs.mate_in == mate_in) return 0;

                return -1;
            } else {
                /* Mate for black. RHS will be better for white unless it is a better mate for black. */
                if (rhs.mate_in <= 0 && rhs.mate_in > mate_in) return -1;
                if (rhs.mate_in == mate_in) return 0;

                return 1;
            }
        } else {
            if (mate_in > 0) return -1;
            if (mate_in < 0) return 1;

            throw std::runtime_error("Unexpected mate_in=0 in evaluation comparison!");
        }
    } else {
        if (rhs.has_mate) {
            if (rhs.mate_in < 0) return -1;
            if (rhs.mate_in > 0) return 1;

            throw std::runtime_error("Unexpected mate_in=0 in evaluation comparison!");
        } else {
            /* Neither side is mate. This make this much easier. */

            if (rhs.eval < eval) {
                return -1;
            } else if (rhs.eval > eval) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}

void Evaluation::add_noise(float amp) {
    eval += (((float) (rand() % 10) / 10.0f) - 0.5f) * 2.0f * amp;
}

bool Evaluation::operator > (const Evaluation& rhs) const {
    return (compare(rhs) == -1);
}

bool Evaluation::operator < (const Evaluation& rhs) const {
    return (compare(rhs) == 1);
}

bool Evaluation::operator >= (const Evaluation& rhs) const {
    return (compare(rhs) <= 0);
}

bool Evaluation::operator <= (const Evaluation& rhs) const {
    return (compare(rhs) >= 0);
}

bool Evaluation::operator == (const Evaluation& rhs) const {
    return (compare(rhs) == 0);
}
