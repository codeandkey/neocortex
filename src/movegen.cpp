/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "movegen.h"
#include "move.h"
#include "attacks.h"
#include "tt.h"

#include <cassert>

using namespace neocortex;
using namespace neocortex::movegen;

static const int quiet_type_order[] = {
		piece::QUEEN,
		piece::ROOK,
		piece::KNIGHT,
		piece::BISHOP,
		piece::PAWN,
		piece::KING,
};

Generator::Generator(Position& p, Move pv_move, int mode) : stage(0), mode(mode), pv_move(pv_move), position(p), board(p.get_board()) {
	current_attacker = piece::PAWN;
	current_victim = piece::QUEEN;
	current_quiet_index = 0;

	ctm = board.get_color_occ(position.get_color_to_move());
	opp = board.get_color_occ(!position.get_color_to_move());
	white_to_move = (position.get_color_to_move() == piece::WHITE);

	if (mode == NORMAL) {
		stage = STAGE_PV;
	}
	else if (mode == QUIESCENCE) {
		stage = QSTAGE_CAPTURES;
	}
}

void Generator::set_killers(Move* killers) {
	this->killers[0] = killers[0];
	this->killers[1] = killers[1];
}

std::list<Move> Generator::next_batch() {
	switch (stage) {
	case STAGE_PV:
		++stage;
		if (pv_move != Move::null) {
			return std::list<Move>(1, pv_move);
		}
		else {
			return next_batch();
		}
	case QSTAGE_PROMOTIONS:
	case STAGE_PROMOTIONS:
		++stage;
		return pawn_promotions();
	case STAGE_KILLERS:
		++stage;

		if (killers[0] != Move::null) {
			std::list<Move> killer_moves;

			killer_moves.push_back(killers[0]);

			if (killers[1] != Move::null) {
				killer_moves.push_back(killers[1]);
			}

			return killer_moves;
		}

		return next_batch();
	case QSTAGE_CAPTURES:
	case STAGE_CAPTURES:
		if (current_attacker > piece::KING) {
			current_attacker = 0;
			
			if (--current_victim < piece::PAWN) {
				++stage;
				return next_batch();
			}
		}

		return captures(current_attacker++, current_victim);
	case QSTAGE_CASTLES:
	case STAGE_CASTLES:
		++stage;
		return castles();
	case QSTAGE_QUIETS:
	case STAGE_QUIETS:
		if (current_quiet_index < 6) {
			if (current_quiet_index == 5) ++stage;
			return quiets(quiet_type_order[current_quiet_index++]);
		}
		break;
	}

	return std::list<Move>();
}

// TODO: specialized qsearch

/*template <>
std::list<Move> Generator::next_batch<QUIESCENCE>() {
	switch (stage) {
	case QSTAGE_PROMOTIONS:
		++stage;
		return pawn_promotions();
	case QSTAGE_CAPTURES:
		if (current_attacker > piece::QUEEN) {
			current_attacker = 0;

			if (--current_victim < piece::PAWN) {
				++stage;
				return next_batch<NORMAL>();
			}
		}

		return captures(current_attacker++, current_victim);
	case QSTAGE_CHECKS:
		if (current_quiet_index < 6) {
			if (current_quiet_index == 5) ++stage;
			return quiets(quiet_type_order[current_quiet_index++]);
		}
		break;
	default:
		return std::list<Move>();
	}
}*/

std::list<Move> Generator::generate() {
	std::list<Move> batch;
	int end = (mode == NORMAL) ? STAGE_END : QSTAGE_END;
	
	do {
		if (stage == end) {
			return batch;
		}

		batch = next_batch();
	} while (!batch.size());

	return batch;
}

Move Generator::next() {
	if (!batch.size()) {
		batch = generate();
		
		if (!batch.size()) {
			return Move::null;
		}
	}

	Move ret = batch.front();
	batch.pop_front();

	return ret;
}

std::list<Move> Generator::pawn_advances() {
	std::list<Move> output;

	bitboard starting_pawn_rank = white_to_move ? RANK_2 : RANK_7;
	bitboard promote_pawn_rank = white_to_move ? RANK_7 : RANK_2;
	int pawn_dir = white_to_move ? NORTH : SOUTH;

	bitboard pawns = ctm & board.get_piece_occ(piece::PAWN);
	bitboard starting_pawns = starting_pawn_rank & pawns;
	
	if (starting_pawns) {
		bitboard first = bb::shift(starting_pawns, pawn_dir) & ~board.get_global_occ();
		bitboard second = bb::shift(first, pawn_dir) & ~board.get_global_occ();

		while (second) {
			int dst = bb::poplsb(second);
			int src = dst - 2 * pawn_dir;

			output.push_back(Move(src, dst, Move::PAWN_JUMP));
		}
	}

	bitboard nonpromote_pawns = pawns & ~promote_pawn_rank;

	if (nonpromote_pawns) {
		bitboard first = bb::shift(nonpromote_pawns, pawn_dir) & ~board.get_global_occ();

		while (first) {
			int dst = bb::poplsb(first);
			int src = dst - pawn_dir;

			output.push_back(Move(src, dst));
		}
	}

	return output;
}

std::list<Move> Generator::pawn_promotions() {
	std::list<Move> output;

	/* Add promoting captures and advances */

	int pawn_dir = white_to_move ? NORTH : SOUTH;
	int left_capture_dir = white_to_move ? NORTHWEST : SOUTHWEST;
	int right_capture_dir = white_to_move ? NORTHEAST : SOUTHEAST;
	bitboard promote_pawn_rank = white_to_move ? RANK_7 : RANK_2;

	bitboard pawns = ctm & board.get_piece_occ(piece::PAWN) & promote_pawn_rank;

	if (pawns) {
		bitboard left_captures = bb::shift(pawns & ~FILE_A, left_capture_dir) & opp;

		while (left_captures) {
			int dst = bb::poplsb(left_captures);
			int src = dst - left_capture_dir;

			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::QUEEN));
			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::KNIGHT));
			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::ROOK));
			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::BISHOP));
		}

		bitboard right_captures = bb::shift(pawns & ~FILE_H, right_capture_dir) & opp;

		while (right_captures) {
			int dst = bb::poplsb(right_captures);
			int src = dst - right_capture_dir;

			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::QUEEN));
			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::KNIGHT));
			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::ROOK));
			output.push_back(Move(src, dst, Move::PROMOTION | Move::CAPTURE, piece::BISHOP));
		}
		
		bitboard advances = bb::shift(pawns, pawn_dir) & ~board.get_global_occ();

		while (advances) {
			int dst = bb::poplsb(advances);
			int src = dst - pawn_dir;

			output.push_back(Move(src, dst, Move::PROMOTION, piece::QUEEN));
			output.push_back(Move(src, dst, Move::PROMOTION, piece::KNIGHT));
			output.push_back(Move(src, dst, Move::PROMOTION, piece::ROOK));
			output.push_back(Move(src, dst, Move::PROMOTION, piece::BISHOP));
		}
	}

	return output;
}

std::list<Move> Generator::captures(int attacker, int victim) {
	assert(attacker >= 0 && attacker < 6);
	assert(victim >= 0 && victim < 6);

	std::list<Move> output;

	bitboard victims = opp & board.get_piece_occ(victim);

	if (attacker == piece::PAWN && victim == piece::PAWN) {
		victims |= position.en_passant_mask();
	}

	switch (attacker) {
		case piece::PAWN:
		{
			int left_capture_dir = white_to_move ? NORTHWEST : SOUTHWEST;
			int right_capture_dir = white_to_move ? NORTHEAST : SOUTHEAST;
			bitboard promote_pawn_rank = white_to_move ? RANK_7 : RANK_2;
			bitboard pawns = ctm & board.get_piece_occ(piece::PAWN) & ~promote_pawn_rank;

			if (pawns) {
				bitboard left_captures = bb::shift(pawns & ~FILE_A, left_capture_dir) & victims;

				while (left_captures) {
					int dst = bb::poplsb(left_captures);
					int src = dst - left_capture_dir;

					output.push_back(Move(src, dst, Move::CAPTURE));
				}

				bitboard right_captures = bb::shift(pawns & ~FILE_H, right_capture_dir) & victims;

				while (right_captures) {
					int dst = bb::poplsb(right_captures);
					int src = dst - right_capture_dir;

					output.push_back(Move(src, dst, Move::CAPTURE));
				}
			}

			break;
		}

		case piece::BISHOP:
		{
			bitboard bishops = ctm & board.get_piece_occ(piece::BISHOP);

			while (bishops) {
				int src = bb::poplsb(bishops);
				bitboard attacked = victims & attacks::bishop(src, board.get_global_occ());

				while (attacked) {
					int dst = bb::poplsb(attacked);
					output.push_back(Move(src, dst, Move::CAPTURE));
				}
			}

			break;
		}

		case piece::KNIGHT:
		{
			bitboard knights = ctm & board.get_piece_occ(piece::KNIGHT);

			while (knights) {
				int src = bb::poplsb(knights);
				bitboard attacked = victims & attacks::knight(src);

				while (attacked) {
					int dst = bb::poplsb(attacked);
					output.push_back(Move(src, dst, Move::CAPTURE));
				}
			}

			break;
		}

		case piece::ROOK:
		{
			bitboard rooks = ctm & board.get_piece_occ(piece::ROOK);

			while (rooks) {
				int src = bb::poplsb(rooks);
				bitboard attacked = victims & attacks::rook(src, board.get_global_occ());

				while (attacked) {
					int dst = bb::poplsb(attacked);
					output.push_back(Move(src, dst, Move::CAPTURE));
				}
			}

			break;
		}

		case piece::QUEEN:
		{
			bitboard queens = ctm & board.get_piece_occ(piece::QUEEN);

			while (queens) {
				int src = bb::poplsb(queens);
				bitboard attacked = victims & attacks::queen(src, board.get_global_occ());

				while (attacked) {
					int dst = bb::poplsb(attacked);
					output.push_back(Move(src, dst, Move::CAPTURE));
				}
			}

			break;
		}

		case piece::KING:
		{
			bitboard kings = ctm & board.get_piece_occ(piece::KING);

			while (kings) {
				int src = bb::poplsb(kings);
				bitboard attacked = victims & attacks::king(src);

				while (attacked) {
					int dst = bb::poplsb(attacked);
					output.push_back(Move(src, dst, Move::CAPTURE));
				}
			}

			break;
		}
	}

	return output;
}

std::list<Move> Generator::quiets(int ptype) {
	assert(ptype >= 0 && ptype < 6);

	std::list<Move> output;

	switch (ptype) {
		case piece::PAWN:
			return pawn_advances();

		case piece::BISHOP:
		{
			bitboard bishops = ctm & board.get_piece_occ(piece::BISHOP);

			while (bishops) {
				int src = bb::poplsb(bishops);
				bitboard moves = attacks::bishop(src, board.get_global_occ()) & ~board.get_global_occ();

				while (moves) {
					int dst = bb::poplsb(moves);
					output.push_back(Move(src, dst));
				}
			}

			break;
		}

		case piece::KNIGHT:
		{
			bitboard knights = ctm & board.get_piece_occ(piece::KNIGHT);

			while (knights) {
				int src = bb::poplsb(knights);
				bitboard moves = attacks::knight(src) & ~board.get_global_occ();

				while (moves) {
					int dst = bb::poplsb(moves);
					output.push_back(Move(src, dst));
				}
			}

			break;
		}

		case piece::ROOK:
		{
			bitboard rooks = ctm & board.get_piece_occ(piece::ROOK);

			while (rooks) {
				int src = bb::poplsb(rooks);
				bitboard moves = attacks::rook(src, board.get_global_occ()) & ~board.get_global_occ();

				while (moves) {
					int dst = bb::poplsb(moves);
					output.push_back(Move(src, dst));
				}
			}

			break;
		}

		case piece::QUEEN:
		{
			bitboard queens = ctm & board.get_piece_occ(piece::QUEEN);

			while (queens) {
				int src = bb::poplsb(queens);
				bitboard moves = attacks::queen(src, board.get_global_occ()) & ~board.get_global_occ();

				while (moves) {
					int dst = bb::poplsb(moves);
					output.push_back(Move(src, dst));
				}
			}

			break;
		}

		case piece::KING:
		{
			bitboard kings = ctm & board.get_piece_occ(piece::KING);

			while (kings) {
				int src = bb::poplsb(kings);
				bitboard moves = attacks::king(src) & ~board.get_global_occ();

				while (moves) {
					int dst = bb::poplsb(moves);
					output.push_back(Move(src, dst));
				}
			}

			break;
		}
	}

	return output;
}

std::list<Move> Generator::castles() {
	std::list<Move> output;

	if (position.check()) return output; /* no castling out of check! */

	/* bit 0: kingside, bit 1: queenside */
	int rights = white_to_move ? position.castle_rights() & 0x3 : position.castle_rights() >> 2;

	bitboard rank = white_to_move ? RANK_1 : RANK_8;
	int rank_num = white_to_move ? 0 : 7;

	bitboard kingside_files = FILE_F | FILE_G;
	bitboard queenside_files = FILE_B | FILE_C | FILE_D;

	bitboard kingside_noattack_files = FILE_F | FILE_G;
	bitboard queenside_noattack_files = FILE_C | FILE_D;

	if (rights & 1) {
		/* Check occ is good*/
		if (!(board.get_global_occ() & rank & kingside_files)) {
			/* Check noattack is good */
			if (!(position.get_current_attacks(!position.get_color_to_move()) & kingside_noattack_files & rank)) {
				output.push_back(Move(square::at(rank_num, 4), square::at(rank_num, 6), Move::CASTLE_KINGSIDE));
			}
		}
	}

	if (rights & 2) {
		/* Check occ is good*/
		if (!(board.get_global_occ() & rank & queenside_files)) {
			/* Check noattack is good */
			if (!(position.get_current_attacks(!position.get_color_to_move()) & queenside_noattack_files & rank)) {
				output.push_back(Move(square::at(rank_num, 4), square::at(rank_num, 2), Move::CASTLE_QUEENSIDE));
			}
		}
	}

	return output;
}

std::list<std::list<Move>> Generator::generate_perft() {
	std::list<std::list<Move>> output;

	output.push_back(pawn_promotions());
	
	for (int v = piece::QUEEN; v >= 0; --v) {
		for (int at = piece::PAWN; at <= piece::KING; ++at) {
			output.push_back(captures(at, v));
		}
	}

	output.push_back(castles());

	for (int i = 0; i < 6; ++i) {
		output.push_back(quiets(quiet_type_order[i]));
	}

	return output;
}
