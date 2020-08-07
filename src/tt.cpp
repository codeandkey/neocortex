/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "tt.h"
#include "log.h"

#include <cassert>
#include <vector>

using namespace neocortex;

static std::vector<tt::entry> tt_vector;

void tt::init(size_t mib) {
	assert(mib >= 16);

	tt_vector.clear();
	tt_vector.resize(((size_t) mib * 1024 * 1024) / sizeof(tt::entry));
	neocortex_debug("Initialized %d MiB hash with %d entries\n", mib, tt_vector.size());
}

tt::entry* tt::lookup(zobrist::Key key) {
	return &tt_vector[key % tt_vector.size()];
}