/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "position.h"
#include "util.h"
#include "attacks.h"

#include "log.h"

#include <cassert>

using namespace neocortex;

Position::Position(std::string fen) {
	std::vector<std::string> fields = util::split(fen, ' ');

	if (fields.size() != 6) {
		throw util::fmterr("Invalid FEN: expected 6 fields, parsed %d", fields.size());
	}

	board = Board(fields[0]);
	color_to_move = piece::color_from_uci(fields[1][0]);
	
	State first_state;

	first_state.castle_rights = 0;

	for (auto c : fields[2]) {
		switch (c) {
		case 'K':
			first_state.castle_rights |= CASTLE_WHITE_K;
			break;
		case 'Q':
			first_state.castle_rights |= CASTLE_WHITE_Q;
			break;
		case 'k':
			first_state.castle_rights |= CASTLE_BLACK_K;
			break;
		case 'q':
			first_state.castle_rights |= CASTLE_BLACK_Q;
			break;
		}
	}

	first_state.en_passant_square = square::from_uci(fields[3]);
	first_state.halfmove_clock = std::stoi(fields[4]);
	first_state.fullmove_number = std::stoi(fields[5]);

	first_state.captured_piece = piece::null;
	first_state.captured_square = piece::null;
	first_state.last_move = Move::null;

	generate_attack_map(piece::WHITE, first_state.attacks[piece::WHITE]);
	generate_attack_map(piece::BLACK, first_state.attacks[piece::BLACK]);

	first_state.key = board.get_tt_key();
	first_state.key ^= zobrist::en_passant(first_state.en_passant_square);
	first_state.key ^= zobrist::castle(first_state.castle_rights);

	if (color_to_move == piece::BLACK) {
		first_state.key ^= zobrist::black_to_move();
	}

	ply.push_back(first_state);
}

std::string Position::to_fen() {
	assert(ply.size());

	std::string output;

	output += board.to_uci() + ' ';
	output += piece::color_to_uci(color_to_move);
	output += ' ';

	if (!ply.back().castle_rights) {
		output += '-';
	} else {
		if (ply.back().castle_rights & CASTLE_WHITE_K) output += 'K';
		if (ply.back().castle_rights & CASTLE_WHITE_Q) output += 'Q';
		if (ply.back().castle_rights & CASTLE_BLACK_K) output += 'k';
		if (ply.back().castle_rights & CASTLE_BLACK_Q) output += 'q';
	}

	output += ' ';

	output += square::to_uci(ply.back().en_passant_square) + ' ';
	output += std::to_string(ply.back().halfmove_clock) + ' ';
	output += std::to_string(ply.back().fullmove_number);

	return output;
}

Board& Position::get_board() {
	return board;
}

bool Position::make_move(Move move) {
	assert(move.is_valid());

	struct State last_state = ply.back(), next_state = last_state;

	next_state.last_move = move;
	
	if (color_to_move == piece::BLACK) {
		next_state.fullmove_number++;
	}

	int dst_piece = board.get_piece(move.dst());
	int src_piece = board.get_piece(move.src());

	next_state.halfmove_clock++;

	if (piece::type(src_piece) == piece::PAWN) {
		next_state.halfmove_clock = 0;
	}

	/* Locate sliding pieces that will require an A/D update. */
	bitboard rq_ad_update = attacks::rook(move.src(), board.get_global_occ()) & (board.get_piece_occ(piece::ROOK) | board.get_piece_occ(piece::QUEEN));
	bitboard bq_ad_update = attacks::bishop(move.src(), board.get_global_occ()) & (board.get_piece_occ(piece::BISHOP) | board.get_piece_occ(piece::QUEEN));

	bitboard all_ad_update = rq_ad_update | bq_ad_update;

	/* Remove attacks from pieces that will receive an update. */
	bitboard all_ad_update_tmp = all_ad_update;
	while (all_ad_update_tmp) {
		remove_attacks(bb::poplsb(all_ad_update_tmp), next_state.attacks);
	}

	/* Also remove attacks from the piece that will be moved. */
	remove_attacks(bb::mask(move.src()), next_state.attacks);

	/* Add the destination square to the list of AD updates */
	all_ad_update |= bb::mask(move.dst());

	/* Perform castling moves */
	if (move.get(Move::CASTLE_KINGSIDE)) {
		int rights = (color_to_move == piece::WHITE) ? 0x3 : 0xC;
		int rank = (color_to_move == piece::WHITE) ? 0 : 7;

		/* the king will be moved during promotion checking, so just move the rook */
		board.place(square::at(rank, 5), board.remove(square::at(rank, 7)));

		/* add the moved rook to the AD update, and remove the update on the old square */
		all_ad_update |= bb::mask(square::at(rank, 5));
		all_ad_update &= ~(bb::mask(square::at(rank, 7)));

		next_state.castle_rights &= ~rights;
	}

	if (move.get(Move::CASTLE_QUEENSIDE)) {
		int rights = (color_to_move == piece::WHITE) ? 0x3 : 0xC;
		int rank = (color_to_move == piece::WHITE) ? 0 : 7;

		board.place(square::at(rank, 3), board.remove(square::at(rank, 0)));

		/* add the moved rook to the AD update, and remove the update on the old square */
		all_ad_update |= bb::mask(square::at(rank, 3));
		all_ad_update &= ~(bb::mask(square::at(rank, 0)));

		next_state.castle_rights &= ~rights;
	}

	/* Revoke castling rights */
	if (piece::type(src_piece) == piece::KING) {
		int rights = (color_to_move == piece::WHITE) ? 0x3 : 0xC;
		next_state.castle_rights &= ~rights;
	}

	if (move.src() == 0) {
		next_state.castle_rights &= ~CASTLE_WHITE_Q;
	}

	if (move.src() == 7) {
		next_state.castle_rights &= ~CASTLE_WHITE_K;
	}

	if (move.src() == 56) {
		next_state.castle_rights &= ~CASTLE_BLACK_Q;
	}

	if (move.src() == 63) {
		next_state.castle_rights &= ~CASTLE_BLACK_K;
	}

	/* Perform captures */
	if (move.get(Move::CAPTURE)) {
		if (dst_piece == piece::null) {
			/* En-passant capture */
			assert(piece::type(src_piece) == piece::PAWN);
			assert(move.dst() == last_state.en_passant_square);

			int psq = (color_to_move == piece::WHITE) ? move.dst() + SOUTH : move.dst() + NORTH;

			/* Drop captured piece attacks */
			remove_attacks(psq, next_state.attacks);

			next_state.captured_piece = board.remove(psq);
			next_state.captured_square = psq;
		} else {
			assert(piece::is_valid(dst_piece));

			/* Drop captured piece attacks */
			remove_attacks(move.dst(), next_state.attacks);

			next_state.captured_piece = board.remove(move.dst());
			next_state.captured_square = move.dst();
		}

		next_state.halfmove_clock = 0;
	} else {
		next_state.captured_piece = piece::null;
		next_state.captured_square = square::null;
	}

	/* Update en passant */
	if (move.get(Move::PAWN_JUMP)) {
		int epsq = (color_to_move == piece::WHITE) ? move.dst() + SOUTH : move.dst() + NORTH;
		next_state.en_passant_square = epsq;
	}
	else {
		next_state.en_passant_square = square::null;
	}

	/* Perform promotion and final move */
	if (move.get(Move::PROMOTION)) {
		board.remove(move.src());
		board.place(move.dst(), piece::make_piece(color_to_move, move.ptype()));
	} else {
		board.place(move.dst(), board.remove(move.src()));
	}

	/* Update AD map for all required pieces */
	while (all_ad_update) {
		add_attacks(bb::poplsb(all_ad_update), next_state.attacks);
	}

	/* Move made, update color to move and push state */
	color_to_move = !color_to_move;

	ply.push_back(next_state);

	/* Check if color that moved is in check */
	int king_loc = bb::getlsb(board.get_color_occ(!color_to_move) & board.get_piece_occ(piece::KING));

	if (next_state.attacks[color_to_move][king_loc]) {
		/* King is in check -- illegal move */
		return false;
	}

	/* update zobrist key */
	ply.back().key = board.get_tt_key();
	ply.back().key ^= zobrist::en_passant(ply.back().en_passant_square);
	ply.back().key ^= zobrist::castle(castle_rights());

	if (color_to_move == piece::BLACK) {
		ply.back().key ^= zobrist::black_to_move();
	}

	return true;
}

void Position::unmake_move(Move move) {
	assert(ply.size() > 1);
	assert(ply.back().last_move == move);

	State last_state = ply.back();
	ply.pop_back();

	color_to_move = !color_to_move;

	/* Unmake castling moves */
	if (move.get(Move::CASTLE_KINGSIDE)) {
		int rank = (color_to_move == piece::WHITE) ? 0 : 7;

		/* the king will be moved during promotion checking, so just move the rook */
		board.place(square::at(rank, 7), board.remove(square::at(rank, 5)));
	}

	if (move.get(Move::CASTLE_QUEENSIDE)) {
		int rank = (color_to_move == piece::WHITE) ? 0 : 7;

		board.place(square::at(rank, 0), board.remove(square::at(rank, 3)));
	}

	if (move.get(Move::PROMOTION)) {
		board.remove(move.dst());
		board.place(move.src(), piece::make_piece(color_to_move, piece::PAWN));
	} else {
		board.place(move.src(), board.remove(move.dst()));
	}

	if (move.get(Move::CAPTURE)) {
		board.place(last_state.captured_square, last_state.captured_piece);
	}
}

bitboard Position::en_passant_mask() {
	int sq = ply.back().en_passant_square;
	return (sq == square::null) ? 0ULL : bb::mask(sq);
}

void Position::generate_attack_map(int color, int* dst) {
	assert(color == piece::WHITE || color == piece::BLACK);

	for (int i = 0; i < 64; ++i) {
		dst[i] = 0;
	}

	bitboard asqlist = 0ULL;

	int left_dir = (color == piece::WHITE) ? NORTHWEST : SOUTHWEST;
	int right_dir = (color == piece::WHITE) ? NORTHEAST : SOUTHEAST;

	bitboard queens = board.get_color_occ(color) & board.get_piece_occ(piece::QUEEN);

	while (queens) {
		asqlist = attacks::queen(bb::poplsb(queens), board.get_global_occ());

		while (asqlist) {
			dst[bb::poplsb(asqlist)]++;
		}
	}

	bitboard rooks = board.get_color_occ(color) & board.get_piece_occ(piece::ROOK);

	while (rooks) {
		asqlist = attacks::rook(bb::poplsb(rooks), board.get_global_occ());

		while (asqlist) {
			dst[bb::poplsb(asqlist)]++;
		}
	}

	bitboard bishops = board.get_color_occ(color) & board.get_piece_occ(piece::BISHOP);

	while (bishops) {
		asqlist = attacks::bishop(bb::poplsb(bishops), board.get_global_occ());

		while (asqlist) {
			dst[bb::poplsb(asqlist)]++;
		}
	}

	bitboard knights = board.get_color_occ(color) & board.get_piece_occ(piece::KNIGHT);

	while (knights) {
		asqlist = attacks::knight(bb::poplsb(knights));

		while (asqlist) {
			dst[bb::poplsb(asqlist)]++;
		}
	}

	bitboard pawns = board.get_color_occ(color) & board.get_piece_occ(piece::PAWN);

	asqlist = bb::shift(pawns & ~FILE_A, left_dir);

	while (asqlist) {
		dst[bb::poplsb(asqlist)]++;
	}

	asqlist = bb::shift(pawns & ~FILE_H, right_dir);

	while (asqlist) {
		dst[bb::poplsb(asqlist)]++;
	}

	bitboard kings = board.get_color_occ(color) & board.get_piece_occ(piece::KING);

	while (kings) {
		asqlist = attacks::king(bb::poplsb(kings));

		while (asqlist) {
			dst[bb::poplsb(asqlist)]++;
		}
	}
}

bool Position::square_is_attacked(int sq, int color) {
	assert(square::is_valid(sq));
	assert(color == piece::WHITE || color == piece::BLACK);

	int left_dir = (color == piece::WHITE) ? SOUTHWEST : NORTHWEST;
	int right_dir = (color == piece::WHITE) ? SOUTHEAST : NORTHEAST;

	bitboard diag_attacks = attacks::bishop(sq, board.get_global_occ());
	bitboard rank_attacks = attacks::rook(sq, board.get_global_occ());

	if ((diag_attacks | rank_attacks) & board.get_color_occ(color) & board.get_piece_occ(piece::QUEEN)) {
		return true;
	}

	if (diag_attacks & board.get_color_occ(color) & board.get_piece_occ(piece::BISHOP)) {
		return true;
	}

	if (rank_attacks & board.get_color_occ(color) & board.get_piece_occ(piece::ROOK)) {
		return true;
	}

	if (attacks::knight(sq) & board.get_color_occ(color) & board.get_piece_occ(piece::KNIGHT)) {
		return true;
	}

	if (attacks::king(sq) & board.get_color_occ(color) & board.get_piece_occ(piece::KING)) {
		return true;
	}

	bitboard sq_mask = bb::mask(sq);
	bitboard pawn_mask = bb::shift(sq_mask & ~FILE_A, left_dir) | bb::shift(sq_mask & ~FILE_H, right_dir);

	if (pawn_mask & board.get_color_occ(color) & board.get_piece_occ(piece::PAWN)) {
		return true;
	}

	return false;
}

zobrist::Key Position::get_tt_key() {
	return ply.back().key;
}

bool Position::check() {
	return ply.back().attacks[!color_to_move][king_loc(color_to_move)];
}

bool Position::quiet() {
	return !(check() || ply.back().last_move.get(Move::CAPTURE | Move::PROMOTION));
}

int Position::castle_rights() {
	return ply.back().castle_rights;
}

int Position::num_repetitions() {
	int res = 0;

	for (size_t i = 0; i < ply.size(); ++i) {
		if (ply[i].key == ply.back().key) {
			++res;
		}
	}

	return res;
}

int Position::halfmove_clock() {
	return ply.back().halfmove_clock;
}

std::string Position::game_string() {
	std::string output;

	for (int i = (int) ply.size() - 1; i > 0; --i) {
		output = ply[i].last_move.to_uci() + " " + output;
	}

	return output;
}

bitboard Position::get_attacks(int sq, int& col) {
	int pc = board.get_piece(sq);

	if (pc == piece::null) {
		col = piece::WHITE;
		return 0ULL;
	}

	col = piece::color(pc);

	switch (piece::type(pc)) {
	case piece::QUEEN:
		return attacks::queen(sq, board.get_global_occ());
	case piece::ROOK:
		return attacks::rook(sq, board.get_global_occ());
	case piece::BISHOP:
		return attacks::bishop(sq, board.get_global_occ());
	case piece::KNIGHT:
		return attacks::knight(sq);
	case piece::KING:
		return attacks::king(sq);
	case piece::PAWN:
		{
			int pcolor = piece::color(pc);
			int dir_left = (pcolor == piece::WHITE) ? NORTHWEST : SOUTHWEST;
			int dir_right = (pcolor == piece::WHITE) ? NORTHEAST : SOUTHEAST;

			bitboard output = 0ULL;

			if (square::file(sq) > 0) {
				output |= bb::shift(bb::mask(sq), dir_left);
			}

			if (square::file(sq) < 7) {
				output |= bb::shift(bb::mask(sq), dir_right);
			}

			return output;
		}
	}
}

void Position::add_attacks(bitboard sqs, int** dst) {
	while (sqs) {
		int col;
		bitboard atmask = get_attacks(bb::poplsb(sqs), col);

		while (atmask) {
			dst[col][bb::poplsb(atmask)]++;
		}
	}
}

void Position::remove_attacks(bitboard sqs, int** dst) {
	while (sqs) {
		int col;
		bitboard atmask = get_attacks(bb::poplsb(sqs), col);

		while (atmask) {
			dst[col][bb::poplsb(atmask)]--;
		}
	}
}
