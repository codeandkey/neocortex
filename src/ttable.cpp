#include "ttable.h"

#include <cstring>

using namespace nc2;

static search::Result* _nc2_ttable_list[ttable::TTABLE_WIDTH];

search::Result* ttable::lookup(Position* p) {
    search::Result* entry = _nc2_ttable_list[p->get_ttable_key() % ttable::TTABLE_WIDTH];

    if (entry && entry->check_position(p)) {
        return entry;
    }

    return NULL;
}

void ttable::store(Position* p, search::Result res) {
    search::Result** entry = _nc2_ttable_list + (p->get_ttable_key() % ttable::TTABLE_WIDTH);

    if (!*entry) *entry = new search::Result();
    (*entry)->update(res);
}
