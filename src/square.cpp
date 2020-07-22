#include "square.h"

#include <cassert>

using namespace pine;

int square::at(int rank, int file) {
	assert(rank >= 0 && rank < 8 && file >= 0 && file < 8);

	return rank * 8 + file;
}

int square::from_uci(std::string uci) {
	if (uci == "-") {
		return square::null;
	} else {
		assert(uci.size() == 2);
		return at(uci[1] - '1', uci[0] - 'a');
	}
}

int square::rank(int sq) {
	assert(square::is_valid(sq));

	return sq >> 3;
}

int square::file(int sq) {
	assert(square::is_valid(sq));

	return sq & 7;
}

std::string square::to_uci(int sq) {
	if (sq == square::null) {
		return "-";
	}

	std::string output;

	output += 'a' + file(sq);
	output += '1' + rank(sq);

	return output;
}

bool square::is_valid(int sq) {
	return sq >= 0 && sq < 64;
}