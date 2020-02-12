#include "ttable.h"

using namespace nc2;

static Result* _nc2_ttable_list[ttable::TTABLE_WIDTH];

Result* ttable::lookup(u64 key) {
    Result* entry = _nc2_ttable_list[key % ttable::TTABLE_WIDTH];

    if (entry && entry->get_key() == key) {
        return entry;
    }

    return NULL;
}

void ttable::store(Result& res) {
    Result** entry = _nc2_ttable_list + (res.get_key() % ttable::TTABLE_WIDTH);

    if (!*entry) {
        (*entry) = new Result(res);
    } else {
        (*entry)->update(res);
    }
}
