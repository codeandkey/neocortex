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

using namespace neocortex;

Board::Board() {
	for (int i = 0; i < 64; ++i) {
		state[i] = -1;

		ad[piece::WHITE][i] = 0;
		ad[piece::BLACK][i] = 0;
	}

	for (int c = 0; c < 2; ++c) {
		color_occ[c] = 0;
		amask[c] = 0;
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

	bitboard updates = 0;

	updates |= attacks::rook(sq, global_occ) & (piece_occ[piece::ROOK] | piece_occ[piece::QUEEN]);
	updates |= attacks::bishop(sq, global_occ) & (piece_occ[piece::BISHOP] | piece_occ[piece::QUEEN]);

	bitboard updates_tmp = updates;

	while (updates_tmp) {
		remove_attacks(bb::poplsb(updates_tmp));
	}

	state[sq] = p;

	add_attacks(sq);

	bitboard mask = bb::mask(sq);

	global_occ ^= mask;
	piece_occ[piece::type(p)] ^= mask;
	color_occ[piece::color(p)] ^= mask;
	key ^= zobrist::piece(sq, p);

	while (updates) {
		add_attacks(bb::poplsb(updates));
	}
}

int Board::remove(int sq) {
	assert(square::is_valid(sq));
	assert(piece::is_valid(state[sq]));

	bitboard updates = 0;

	updates |= attacks::rook(sq, global_occ) & (piece_occ[piece::ROOK] | piece_occ[piece::QUEEN]);
	updates |= attacks::bishop(sq, global_occ) & (piece_occ[piece::BISHOP] | piece_occ[piece::QUEEN]);

	bitboard updates_tmp = updates;

	while (updates_tmp) {
		remove_attacks(bb::poplsb(updates_tmp));
	}

	remove_attacks(sq);

	int res = state[sq];

	bitboard mask = bb::mask(sq);

	global_occ ^= mask;
	piece_occ[piece::type(res)] ^= mask;
	color_occ[piece::color(res)] ^= mask;
	key ^= zobrist::piece(sq, res);

	state[sq] = piece::null;

	while (updates) {
		add_attacks(bb::poplsb(updates));
	}

	return res;
}

int Board::replace(int sq, int p) {
	assert(square::is_valid(sq));
	assert(piece::is_valid(state[sq]));

	remove_attacks(sq);

	int res = state[sq];

	bitboard mask = bb::mask(sq);

	piece_occ[piece::type(res)] ^= mask;
	color_occ[piece::color(res)] ^= mask;
	piece_occ[piece::type(p)] ^= mask;
	color_occ[piece::color(p)] ^= mask;

	key ^= zobrist::piece(sq, res);
	key ^= zobrist::piece(sq, p);

	state[sq] = p;
	add_attacks(sq);

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

void Board::add_attacks(int sq) {
	int col = piece::color(state[sq]);
	int pt = piece::type(state[sq]);

	bitboard att = 0;

	switch (pt) {
	case piece::PAWN:
	{
		int left_dir = (col == piece::WHITE) ? NORTHWEST : SOUTHWEST;
		int right_dir = (col == piece::WHITE) ? NORTHEAST : SOUTHEAST;

		bitboard left_att = bb::shift(bb::mask(sq) & ~FILE_A, left_dir);
		bitboard right_att = bb::shift(bb::mask(sq) & ~FILE_H, right_dir);

		att = left_att | right_att;
	}
	break;
	case piece::BISHOP:
		att = attacks::bishop(sq, global_occ);
		break;
	case piece::KNIGHT:
		att = attacks::knight(sq);
		break;
	case piece::ROOK:
		att = attacks::rook(sq, global_occ);
		break;
	case piece::QUEEN:
		att = attacks::queen(sq, global_occ);
		break;
	case piece::KING:
		att = attacks::king(sq);
		break;
	}

	amask[col] |= att;

	while (att) {
		ad[col][bb::poplsb(att)]++;
	}
}

void Board::remove_attacks(int sq) {
	int col = piece::color(state[sq]);
	int pt = piece::type(state[sq]);

	bitboard att = 0;

	switch (pt) {
	case piece::PAWN:
	{
		int left_dir = (col == piece::WHITE) ? NORTHWEST : SOUTHWEST;
		int right_dir = (col == piece::WHITE) ? NORTHEAST : SOUTHEAST;

		bitboard left_att = bb::shift(bb::mask(sq) & ~FILE_A, left_dir);
		bitboard right_att = bb::shift(bb::mask(sq) & ~FILE_H, right_dir);

		att = left_att | right_att;
	}
	break;
	case piece::BISHOP:
		att = attacks::bishop(sq, global_occ);
		break;
	case piece::KNIGHT:
		att = attacks::knight(sq);
		break;
	case piece::ROOK:
		att = attacks::rook(sq, global_occ);
		break;
	case piece::QUEEN:
		att = attacks::queen(sq, global_occ);
		break;
	case piece::KING:
		att = attacks::king(sq);
		break;
	}

	while (att) {
		int sq = bb::poplsb(att);

		if (!--ad[col][sq]) {
			amask[col] &= ~bb::mask(sq);
		}
	}
}