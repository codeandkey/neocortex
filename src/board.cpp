/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "board.h"
#include "util.h"

#include "log.h"

#include <cassert>
#include <cctype>
#include <vector>

using namespace pine;

Board::Board() {
	for (int i = 0; i < 64; ++i) {
		state[i] = -1;
	}

	for (int c = 0; c < 2; ++c) {
		color_occ[c] = 0;
	}

	for (int p = 0; p < 6; ++p) {
		piece_occ[p] = 0;
	}

	key = 0;
	global_occ = 0;
}

Board::Board(std::string uci) : Board() {
	std::vector<std::string> ranks = util::split(uci, '/');

	if (ranks.size() != 8) {
		throw util::fmterr("Invalid UCI: expected 8 ranks, parsed %d", ranks.size());
	}

	int r = 8;
	for (auto rank : ranks) {
		--r;
		int f = 0;

		for (auto c : rank) {
			if (isdigit(c)) {
				for (int j = 0; j < (c - '0'); ++j) {
					if (f >= 8) {
						throw util::fmterr("Invalid UCI: overflow in %s", rank.c_str());
					}

					state[square::at(r, f++)] = piece::null;
				}
			} else {
				if (f >= 8) {
					throw util::fmterr("Invalid UCI: overflow in %s", rank.c_str());
				}

				place(square::at(r, f++), piece::from_uci(c));
			}
		}

		if (f != 8) {
			throw util::fmterr("Invalid UCI: not enough pieces in %s", rank.c_str());
		}
	}
}

Board Board::standard() {
	return Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

void Board::place(int sq, int p) {
	assert(square::is_valid(sq));
	assert(piece::is_valid(p));
	assert(state[sq] == piece::null);

	state[sq] = p;

	bitboard mask = bb::mask(sq);

	global_occ ^= mask;
	piece_occ[piece::type(p)] ^= mask;
	color_occ[piece::color(p)] ^= mask;
	key ^= zobrist::piece(sq, p);
}

int Board::remove(int sq) {
	assert(square::is_valid(sq));
	assert(piece::is_valid(state[sq]));

	int res = state[sq];

	bitboard mask = bb::mask(sq);

	global_occ ^= mask;
	piece_occ[piece::type(res)] ^= mask;
	color_occ[piece::color(res)] ^= mask;
	key ^= zobrist::piece(sq, res);

	state[sq] = piece::null;

	return res;
}

std::string Board::to_uci() {
	std::string output;

	int null_count = 0;

	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f < 8; ++f) {
			int sq = square::at(r, f);

			if (state[sq] == piece::null) {
				null_count++;
			} else {
				if (null_count > 0) {
					output += '0' + null_count;
					null_count = 0;
				}

				output += piece::get_uci(state[sq]);
			}
		}

		if (null_count > 0) {
			output += '0' + null_count;
			null_count = 0;
		}

		if (r) output += '/';
	}

	return output;
}

std::string Board::to_pretty() {
	std::string output;

	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f < 8; ++f) {
			int sq = square::at(r, f);

			if (state[sq] == piece::null) {
				output += '.';
			} else {
				output += piece::get_uci(state[sq]);
			}
		}

		output += '\n';
	}

	return output;
}

bitboard Board::get_global_occ() {
	return global_occ;
}

bitboard Board::get_color_occ(int col) {
	assert(col == piece::WHITE || col == piece::BLACK);

	return color_occ[col];
}

bitboard Board::get_piece_occ(int ptype) {
	assert(ptype >= 0 && ptype < 6);
	
	return piece_occ[ptype];
}

int Board::get_piece(int sq) {
	assert(square::is_valid(sq));

	return state[sq];
}

zobrist::key Board::get_tt_key() {
	return key;
}
