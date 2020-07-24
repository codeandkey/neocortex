#include "tt.h"
#include "log.h"

#include <cassert>
#include <vector>

using namespace pine;

static std::vector<tt::entry> tt_vector;

void tt::init(size_t mib) {
	assert(mib >= 16);

	tt_vector.clear();
	tt_vector.resize(((size_t) mib * 1024 * 1024) / sizeof(tt::entry));
	pine_debug("Initialized %d MiB hash with %d entries\n", mib, tt_vector.size());
}

tt::entry* tt::lookup(zobrist::key key) {
	return &tt_vector[key % tt_vector.size()];
}