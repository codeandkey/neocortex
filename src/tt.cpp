/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "tt.h"
#include "log.h"

#include <cstdlib>
#include <cassert>
#include <thread>
#include <mutex>
#include <vector>

using namespace neocortex;

static tt::entry* tt_buffer;
static int tt_buffer_size;
static std::mutex tt_mutex;

void tt::init(size_t mib) {
	tt::resize(mib);
}

tt::entry* tt::lookup(zobrist::Key key) {
	return &tt_buffer[key % tt_buffer_size];
}

void tt::resize(int mb) {
	std::lock_guard<std::mutex> lock(tt_mutex);
	if (tt_buffer) delete[] tt_buffer;

	tt_buffer_size = mb * 1024 * 1024 / sizeof(tt::entry);
	tt_buffer = new tt::entry[tt_buffer_size];

	neocortex_debug("Transposition table resized to %d mb\n", mb);
}

void tt::lock() {
	tt_mutex.lock();
}

void tt::unlock() {
	tt_mutex.unlock();
}
