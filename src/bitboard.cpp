#include "bitboard.h"
#include "platform.h"
#include "square.h"

#ifdef PINE_WIN32
#include <intrin.h>
#endif

#include <cassert>

using namespace pine;

std::string bb::to_string(bitboard inp) {
	std::string output;

	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f < 8; ++f) {
			if (bb::mask(square::at(r, f)) & inp) {
				output += '1';
			} else {
				output += '.';
			}
		}

		output += '\n';
	}

	return output;
}

int bb::getlsb(bitboard b) {
	assert(b);

#ifdef PINE_WIN32
	unsigned long pos;
	_BitScanForward64(&pos, b);

	return (int) pos;
#elif defined PINE_LINUX || defined PINE_OSX
	return (int) __builtin_ctzll(b);
#endif
}

int bb::poplsb(bitboard& b) {
	int lsb = getlsb(b);
	b ^= 1ULL << lsb;
	return lsb;
}

bitboard bb::shift(bitboard b, int dir) {
	if (dir > 0) {
		return b << dir;
	} else {
		return b >> -dir;
	}
}

bitboard bb::mask(int sq) {
	assert(square::is_valid(sq));

	return ((bitboard) 1) << sq;
}

int bb::popcount(bitboard b) {
#ifdef PINE_WIN32
	return (int) __popcnt64(b);
#elif defined PINE_LINUX || defined PINE_OSX
	return (int) __builtin_popcountll(b);
#endif
}

bitboard bb::rank(int r) {
	assert(r >= 0 && r < 8);
	return RANK_1 << (8 * r);
}

bitboard bb::file(int f) {
	assert(f >= 0 && f < 8);
	return FILE_A << f;
}