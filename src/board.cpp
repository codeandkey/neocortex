/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "board.h"
#include "eval_consts.h"
#include "nn.h"
#include "util.h"

#include "log.h"

#include <cassert>
#include <cctype>
#include <vector>

using namespace neocortex;

Board::Board() {
	//nn_inputs[0] = new float[nn::INPUT_NODES / 2];
	//nn_inputs[1] = new float[nn::INPUT_NODES / 2];

	for (int i = 0; i < 64; ++i) {
		state[i] = -1;
	}

	for (int c = 0; c < 2; ++c) {
		color_occ[c] = 0;
	}

	for (int p = 0; p < 6; ++p) {
		piece_occ[p] = 0;
	}

	for (int c = 0; c < 2; ++c) {
		for (int i = 0; i < 20480; ++i) {
			nn_inputs[c][i] = 0.0f;
		}
	}

	key = 0;
	global_occ = 0;
	mat_mg = mat_eg = 0;
	ksq[piece::WHITE] = ksq[piece::BLACK] = square::null;
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

Board::~Board() {
	//delete[] nn_inputs[0];
	//delete[] nn_inputs[1];
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
	mat_mg += eval::MATERIAL_MG_LOOKUP[p];
	mat_eg += eval::MATERIAL_EG_LOOKUP[p];

	// alter NN inputs
	int col = piece::color(p);
	
	if (piece::type(p) == piece::KING) {
		// set inputs for all pieces of this color
		
		for (int i = 0; i < 64; ++i) {
			if (i == sq) continue;
			if (piece::is_valid(state[i]) && piece::color(state[i]) == col) {
				nn_inputs[col][320 * sq + 5 * i + piece::type(state[i])] = 1.0f;
			}
		}

		assert(ksq[col] == square::null);
		ksq[col]= sq;
	} else {
		// alter single input
		if (square::is_valid(ksq[col])) {
			nn_inputs[col][320 * ksq[col] + 5 * sq + piece::type(p)] = 1.0f;
		}
	}
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

	mat_mg -= eval::MATERIAL_MG_LOOKUP[res];
	mat_eg -= eval::MATERIAL_EG_LOOKUP[res];

	// alter NN inputs
	int col = piece::color(res);

	if (piece::type(res) == piece::KING) {
		// set inputs for all pieces of this color

		for (int i = 0; i < 64; ++i) {
			if (i == sq) continue;
			if (piece::is_valid(state[i]) && piece::color(state[i]) == col) {
				nn_inputs[col][320 * sq + 5 * i + piece::type(state[i])] = 0.0f;
			}
		}

		assert(square::is_valid(ksq[col]));
		ksq[col] = square::null;
	}
	else {
		// alter single input
		if (square::is_valid(ksq[col])) {
			nn_inputs[col][320 * ksq[col] + 5 * sq + piece::type(res)] = 0.0f; 
		}
	}

	return res;
}

int Board::replace(int sq, int p) {
	int r = remove(sq);
	place(sq, p);
	return r;
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

bitboard Board::attacks_on(int sq) {
	assert(square::is_valid(sq));

	bitboard white_pawns = piece_occ[piece::PAWN] & color_occ[piece::WHITE];
	bitboard black_pawns = piece_occ[piece::PAWN] & color_occ[piece::BLACK];
	bitboard rooks_queens = piece_occ[piece::ROOK] | piece_occ[piece::QUEEN];
	bitboard bishops_queens = piece_occ[piece::BISHOP] | piece_occ[piece::QUEEN];

	return (attacks::pawn(piece::WHITE, sq) & black_pawns) |
		(attacks::pawn(piece::BLACK, sq) & white_pawns) |
		(attacks::knight(sq) & piece_occ[piece::KNIGHT]) |
		(attacks::bishop(sq, global_occ) & bishops_queens) |
		(attacks::rook(sq, global_occ) & rooks_queens) |
		(attacks::king(sq) & piece_occ[piece::KING]);
}

int Board::guard_value(int sq) {
	int val = 0;
	bitboard att = attacks_on(sq);

	while (att) {
		val += eval::GUARD_VALUES[state[bb::poplsb(att)]];
	}

	return val;
}

bool Board::mask_is_attacked(bitboard mask, int col) {
	bitboard opp = color_occ[col];

	while (mask) {
		if (attacks_on(bb::poplsb(mask)) & opp) return true;
	}

	return false;
}

bitboard Board::all_spans(int col) {
	bitboard out = 0;
	bitboard pawns = piece_occ[piece::PAWN] & color_occ[col];

	while (pawns) {
		int p = bb::poplsb(pawns);

		out |= attacks::pawn_attackspans(col, p);
		out |= attacks::pawn_frontspans(col, p);
	}

	return out;
}

bitboard Board::front_spans(int col) {
	bitboard out = 0;
	bitboard pawns = piece_occ[piece::PAWN] & color_occ[col];

	while (pawns) {
		int p = bb::poplsb(pawns);

		out |= attacks::pawn_frontspans(col, p);
	}

	return out;
}

bitboard Board::attack_spans(int col) {
	bitboard out = 0;
	bitboard pawns = piece_occ[piece::PAWN] & color_occ[col];

	while (pawns) {
		int p = bb::poplsb(pawns);

		out |= attacks::pawn_attackspans(col, p);
	}

	return out;
}

bitboard Board::isolated_pawns(int col) {
	bitboard pawns = piece_occ[piece::PAWN] & color_occ[col];
	bitboard out = 0ULL;

	while (pawns) {
		int p = bb::poplsb(pawns);

		if (!(bb::neighbor_files(p) & pawns)) {
			out |= bb::mask(p);
		}
	}
	
	return out;
}

bitboard Board::backward_pawns(int col) {
	bitboard pawns = piece_occ[piece::PAWN] & color_occ[col];
	bitboard stops = bb::shift(pawns, (col == piece::WHITE) ? NORTH : SOUTH);

	bitboard opp_pawns = piece_occ[piece::PAWN] & color_occ[!col];
	bitboard opp_attacks = bb::shift(opp_pawns & ~FILE_A, (col == piece::WHITE) ? NORTHWEST : SOUTHWEST);

	opp_attacks |= bb::shift(opp_pawns & ~FILE_H, (col == piece::WHITE) ? NORTHEAST : SOUTHEAST);

	stops &= ~attack_spans(col);
	stops &= opp_attacks;

	return bb::shift(stops, (col == piece::WHITE) ? SOUTH : NORTH);
}

bitboard Board::passedpawns(int col) {
	return ~all_spans(!col) & piece_occ[piece::PAWN] & color_occ[col];
}
