#include "ttable.h"

#include <cstring>

using namespace nc2;

static ttable::Entry* _nc2_ttable_list[ttable::TTABLE_WIDTH];

ttable::Entry::Entry(Position p, Evaluation e, int d) : p(p), e(e), depth(d) {}

bool ttable::lookup(Position* p, Evaluation* saved_eval, int mindepth) {
    ttable::Entry* entry = _nc2_ttable_list[p->get_ttable_key() % ttable::TTABLE_WIDTH];

    if (entry && entry->depth >= mindepth && !memcmp(p->get_board(), entry->p.get_board(), 64)) {
        *saved_eval = entry->e;
        return true;
    }

    return false;
}

void ttable::store(Position* p, Evaluation position_eval, int depth) {
    ttable::Entry** entry = _nc2_ttable_list + (p->get_ttable_key() % ttable::TTABLE_WIDTH);

    if (*entry) {
        *entry = new ttable::Entry(*p, position_eval, depth);
    } else {
        (*entry)->p = *p;
        (*entry)->e = position_eval;
        (*entry)->depth = depth;
    }
}
