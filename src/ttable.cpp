#include "ttable.h"

#include <cstring>

using namespace nc2;

static ttable::Entry* _nc2_ttable_list[ttable::TTABLE_WIDTH];

ttable::Entry::Entry(Position p, Evaluation e, int d) : p(p), e(e), depth(d) {}

bool ttable::lookup(Position* p, Evaluation* saved_eval, int* saved_depth) {
    ttable::Entry* entry = _nc2_ttable_list[p->get_ttable_key() % ttable::TTABLE_WIDTH];

    if (entry && !memcmp(p->get_board(), entry->p.get_board(), 64)) {
        *saved_eval = entry->e;
        *saved_depth = entry->depth;
        return true;
    }

    return false;
}

void ttable::store(Position* p, Evaluation position_eval, int depth) {
    ttable::Entry** entry = _nc2_ttable_list + (p->get_ttable_key() % ttable::TTABLE_WIDTH);

    if (*entry) {
        (*entry)->p = *p;
        (*entry)->e = position_eval;
        (*entry)->depth = depth;
    } else {
        *entry = new ttable::Entry(*p, position_eval, depth);
    }
}
