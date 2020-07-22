#include "tt.h"
#include "log.h"

#include <cassert>
#include <vector>

using namespace pine;

static std::vector<tt::entry> tt_vector;
static zobrist::key tt_mask;

void tt::init(int bits) {
	assert(bits > 0 && bits <= 24);

	tt_vector.resize(1ULL << bits, { 0, Move::null });
	pine_debug("Using %d bit transposition table with %d entries\n", bits, 1 << bits);

	tt_mask = (1ULL << (bits + 1)) - 1;
}

tt::entry* tt::lookup(zobrist::key key) {
	return &tt_vector[key & tt_mask];
}