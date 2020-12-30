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
#include <shared_mutex>
#include <mutex>
#include <vector>

using namespace neocortex;

static tt::entry* tt_buffer;
static std::mutex* tt_mutex_buffer;
static int tt_buffer_size;
static std::shared_mutex tt_resize_mutex;

void tt::init(size_t mib) {
	tt::resize(mib);
}

tt::entry* tt::lookup(zobrist::Key key) {
	return &tt_buffer[key % tt_buffer_size];
}

void tt::resize(int mb) {
	std::lock_guard<std::shared_mutex> lock(tt_resize_mutex);
	if (tt_buffer) delete[] tt_buffer;
	if (tt_mutex_buffer) delete[] tt_mutex_buffer;

	tt_buffer_size = mb * 1024 * 1024 / (sizeof(tt::entry) + sizeof(std::mutex));
	tt_buffer = new tt::entry[tt_buffer_size];
	tt_mutex_buffer = new std::mutex[tt_buffer_size];

	neocortex_debug("Transposition table resized to %d mb (%d entries)\n", mb, tt_buffer_size);
}

void tt::lock(zobrist::Key k) {
	tt_mutex_buffer[k % tt_buffer_size].lock();
	tt_resize_mutex.lock_shared();
}

void tt::unlock(zobrist::Key k) {
	tt_mutex_buffer[k % tt_buffer_size].unlock();
	tt_resize_mutex.unlock_shared();
}
