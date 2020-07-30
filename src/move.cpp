/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "bitboard.h"
#include "move.h"
#include "util.h"

#include <cassert>

using namespace pine;

Move::Move(int value) : value(value) {}

Move::Move(int src, int dst, int flags, int ptype) {
	assert(square::is_valid(src));
	assert(square::is_valid(dst));
	assert(ptype >= 0 && ptype < 6);

	value = src | (dst << 6) | (ptype << 12) | flags;
}

Move::Move(std::string uci) {
	if (uci.size() < 4 || uci.size() > 5) {
		throw util::fmterr("Invalid UCI \"%s\": moves must be 4 or 5 characters, parsed %d", uci.c_str(), uci.size());
	}

	int sq_src = square::from_uci(uci.substr(0, 2));
	int sq_dst = square::from_uci(uci.substr(2, 2));
	int new_ptype = (uci.size() == 5) ? piece::type_from_uci(uci[4]) : piece::PAWN;

	value = sq_src | (sq_dst << 6) | (new_ptype << 12);

	if (new_ptype != piece::PAWN) {
		value |= PROMOTION;
	}
}

std::string Move::to_uci() {
	std::string output;

	output += square::to_uci(src());
	output += square::to_uci(dst());

	if (value & PROMOTION) {
		output += piece::type_to_uci(ptype());
	}

	return output;
}

std::string Move::to_pgn(Board& context) {
	assert(is_valid());

	std::string output;

	int srcpiece = context.get_piece(src());
	int type = piece::type(srcpiece);
	int color = piece::color(srcpiece);

	if (value & CASTLE_KINGSIDE) {
		return "O-O";
	} else if (value & CASTLE_QUEENSIDE) {
		return "O-O-O";
	}

	if (type == piece::PAWN) {
		if (value & CAPTURE) {
			output += square::to_uci(src())[0];
		}
	} else {
		output += toupper(piece::type_to_uci(type));

		/* Perform a basic attack lookup for the other pieces. We need only test RNBQ */
		bitboard candidates = context.get_piece_occ(type) & context.get_color_occ(color);

		switch (type) {
		case piece::BISHOP:
			candidates &= attacks::bishop(dst(), context.get_global_occ());
			break;
		case piece::KNIGHT:
			candidates &= attacks::knight(dst());
			break;
		case piece::ROOK:
			candidates &= attacks::rook(dst(), context.get_global_occ());
			break;
		case piece::QUEEN:
			candidates &= attacks::queen(dst(), context.get_global_occ());
			break;
		case piece::KING:
			candidates &= attacks::queen(dst(), context.get_global_occ());
			break;
		}

		/* If there are multiple candidates, try narrowing down. */
		if (bb::popcount(candidates) > 1) {
			int rank = square::rank(src()), file = square::file(src());
			int rank_count = bb::popcount(context.get_color_occ(color) & context.get_piece_occ(type) & bb::rank(rank));
			int file_count = bb::popcount(context.get_color_occ(color) & context.get_piece_occ(type) & bb::file(file));

			if (file_count == 1) {
				output += square::to_uci(src())[0];
			}
			else if (rank_count == 1) {
				output += square::to_uci(src())[1];
			}
			else {
				output += square::to_uci(src())[0];
				output += square::to_uci(src())[1];
			}
		}
	}

	if (value & CAPTURE) {
		output += 'x';
	}

	output += square::to_uci(dst());

	if (value & PROMOTION) {
		output += '=';
		output += toupper(piece::type_to_uci(ptype()));
	}

	return output;
}

bool Move::is_valid() {
	if (!square::is_valid(value & 0x3F)) return false;
	if (!square::is_valid((value >> 6) & 0x3F)) return false;
	if (!piece::is_type((value >> 12) & 7)) return false;

	return true;
}

bool Move::match_uci(std::string uci) {
	Move ucimove(uci);

	if (ucimove.src() != src()) return false;
	if (ucimove.dst() != dst()) return false;
	if (ucimove.get(PROMOTION) != get(PROMOTION)) return false;

	if (get(PROMOTION)) {
		if (ucimove.ptype() != ptype()) return false;
	}

	return true;
}

int Move::src() {
	assert(is_valid());

	return value & 0x3F;
}

int Move::dst() {
	assert(is_valid());

	return (value >> 6) & 0x3F;
}

int Move::ptype() {
	assert(is_valid());

	return (value >> 12) & 7;
}

Move& Move::set(int flag) {
	value |= flag;
	return *this;
}

bool Move::get(int flag) {
	return value & flag;
}

Move::operator std::string() {
	return to_uci();
}

bool Move::operator==(const Move& rhs) {
	return value == rhs.value;
}

bool Move::operator!=(const Move& rhs) {
	return value != rhs.value;
}

PV::PV() {
	len = 0;

	for (int i = 0; i < MAXSIZE; ++i) {
		moves[i] = Move::null;
	}
}

std::string PV::to_string() {
	std::string output;

	for (int i = 0; i < len; ++i) {
		if (i) output += ' ';
		output += moves[i].to_uci();
	}

	return output;
}