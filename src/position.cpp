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

Position::Position() {
	board = Board::standard();

	State first_state;

	first_state.castle_rights = 0xF;
	first_state.en_passant_square = square::null;
	first_state.fullmove_number = 1;
	first_state.halfmove_clock = 0;
	first_state.captured_piece = piece::null;
	first_state.captured_square = piece::null;
	first_state.last_move = Move::null;
	first_state.attacks[piece::WHITE] = squares_attacked_by(piece::WHITE);
	first_state.attacks[piece::BLACK] = squares_attacked_by(piece::BLACK);
	first_state.key = 0;
	first_state.key ^= board.get_tt_key();
	first_state.key ^= zobrist::castle(first_state.castle_rights);

	ply.push_back(first_state);
	color_to_move = piece::WHITE;
}

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
	first_state.attacks[piece::WHITE] = squares_attacked_by(piece::WHITE);
	first_state.attacks[piece::BLACK] = squares_attacked_by(piece::BLACK);

	first_state.key = 0;
	first_state.key ^= board.get_tt_key();
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
	struct State last_state = ply.back(), next_state = last_state;

	if (move == Move::null) {
		/* Perform null move */
		next_state.en_passant_square = square::null;
		next_state.captured_piece = piece::null;
		next_state.last_move = Move::null;
		
		if (color_to_move == piece::BLACK) {
			next_state.fullmove_number++;
		}

		next_state.halfmove_clock++;
		next_state.captured_square = square::null;

		color_to_move = !color_to_move;

		if (color_to_move == piece::BLACK) {
			ply.back().key ^= zobrist::black_to_move();
		}

		ply.push_back(next_state);

		return true;
	}

	/* reset attack masks */
	next_state.attacks[piece::WHITE] = 0ULL;
	next_state.attacks[piece::BLACK] = 0ULL;

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

	/* Perform castling moves */
	if (move.get(Move::CASTLE_KINGSIDE)) {
		int rights = (color_to_move == piece::WHITE) ? 0x3 : 0xC;
		int rank = (color_to_move == piece::WHITE) ? 0 : 7;

		/* the king will be moved during promotion checking, so just move the rook */
		board.place(square::at(rank, 5), board.remove(square::at(rank, 7)));

		next_state.castle_rights &= ~rights;
	}

	if (move.get(Move::CASTLE_QUEENSIDE)) {
		int rights = (color_to_move == piece::WHITE) ? 0x3 : 0xC;
		int rank = (color_to_move == piece::WHITE) ? 0 : 7;

		board.place(square::at(rank, 3), board.remove(square::at(rank, 0)));

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

			next_state.captured_piece = board.remove(psq);
			next_state.captured_square = psq;
		} else {
			assert(piece::is_valid(dst_piece));

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

	/* Move made, update color to move and push state */
	color_to_move = !color_to_move;

	ply.push_back(next_state);

	/* Check if color that moved is in check */
	if (square_is_attacked(bb::getlsb(board.get_color_occ(!color_to_move) & board.get_piece_occ(piece::KING)), color_to_move)) {
		/* King is in check -- illegal move */
		return false;
	}

	/* Update complete attack masks */
	ply.back().attacks[color_to_move] = squares_attacked_by(color_to_move);

	/* TODO: check for illegal castles here */
	ply.back().attacks[!color_to_move] = squares_attacked_by(!color_to_move);

	/* update zobrist key */
	ply.back().key = 0;
	ply.back().key ^= board.get_tt_key();
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

	/* Unmake null move */
	if (move == Move::null) {
		return;
	}

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

bitboard Position::squares_attacked_by(int color) {
	assert(color == piece::WHITE || color == piece::BLACK);

	bitboard output = 0ULL;
	int left_dir = (color == piece::WHITE) ? NORTHWEST : SOUTHWEST;
	int right_dir = (color == piece::WHITE) ? NORTHEAST : SOUTHEAST;

	bitboard queens = board.get_color_occ(color) & board.get_piece_occ(piece::QUEEN);

	while (queens) {
		int src = bb::poplsb(queens);
		output |= attacks::queen(src, board.get_global_occ());
	}

	bitboard rooks = board.get_color_occ(color) & board.get_piece_occ(piece::ROOK);

	while (rooks) {
		int src = bb::poplsb(rooks);
		output |= attacks::rook(src, board.get_global_occ());
	}

	bitboard bishops = board.get_color_occ(color) & board.get_piece_occ(piece::BISHOP);

	while (bishops) {
		int src = bb::poplsb(bishops);
		output |= attacks::bishop(src, board.get_global_occ());
	}

	bitboard knights = board.get_color_occ(color) & board.get_piece_occ(piece::KNIGHT);

	while (knights) {
		int src = bb::poplsb(knights);
		output |= attacks::knight(src);
	}

	bitboard pawns = board.get_color_occ(color) & board.get_piece_occ(piece::PAWN);

	output |= bb::shift(pawns & ~FILE_A, left_dir);
	output |= bb::shift(pawns & ~FILE_A, right_dir);

	bitboard kings = board.get_color_occ(color) & board.get_piece_occ(piece::KING);

	while (kings) {
		int src = bb::poplsb(kings);
		output |= attacks::king(src);
	}

	return output;
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

void Position::get_attackers_defenders(int sq, int& white, int& black) {
	bitboard rook_attacks = attacks::rook(sq, get_board().get_global_occ());
	bitboard bishop_attacks = attacks::bishop(sq, get_board().get_global_occ());
	bitboard queen_attacks = rook_attacks | bishop_attacks;
	bitboard king_attacks = attacks::king(sq);
	bitboard knight_attacks = attacks::knight(sq);

	bitboard mask = bb::mask(sq);
	
	bitboard wp_attacks = bb::shift(mask & ~RANK_1 & ~RANK_2 & ~FILE_H, SOUTHEAST) | bb::shift(mask & ~RANK_1 & ~RANK_2 & ~FILE_A, SOUTHWEST);
	bitboard bp_attacks = bb::shift(mask & ~RANK_7 & ~RANK_8 & ~FILE_H, NORTHEAST) | bb::shift(mask & ~RANK_7 & ~RANK_8 & ~FILE_A, NORTHWEST);
	
	white = black = 0;

	white += bb::popcount(rook_attacks & board.get_piece_occ(piece::ROOK) & board.get_color_occ(piece::WHITE));
	black += bb::popcount(rook_attacks & board.get_piece_occ(piece::ROOK) & board.get_color_occ(piece::BLACK));
	white += bb::popcount(bishop_attacks & board.get_piece_occ(piece::BISHOP) & board.get_color_occ(piece::WHITE));
	black += bb::popcount(bishop_attacks & board.get_piece_occ(piece::BISHOP) & board.get_color_occ(piece::BLACK));
	white += bb::popcount(knight_attacks & board.get_piece_occ(piece::KNIGHT) & board.get_color_occ(piece::WHITE));
	black += bb::popcount(knight_attacks & board.get_piece_occ(piece::KNIGHT) & board.get_color_occ(piece::BLACK));
	white += bb::popcount(queen_attacks & board.get_piece_occ(piece::QUEEN) & board.get_color_occ(piece::WHITE));
	black += bb::popcount(queen_attacks & board.get_piece_occ(piece::QUEEN) & board.get_color_occ(piece::BLACK));
	white += bb::popcount(wp_attacks & board.get_piece_occ(piece::PAWN) & board.get_color_occ(piece::WHITE));
	black += bb::popcount(bp_attacks & board.get_piece_occ(piece::PAWN) & board.get_color_occ(piece::BLACK));
	white += bb::popcount(king_attacks & board.get_piece_occ(piece::KING) & board.get_color_occ(piece::WHITE));
	black += bb::popcount(king_attacks & board.get_piece_occ(piece::KING) & board.get_color_occ(piece::BLACK));
}

bitboard Position::get_current_attacks(int color) {
	return ply.back().attacks[color];
}

zobrist::Key Position::get_tt_key() {
	return ply.back().key;
}

bool Position::check() {
	return (ply.back().attacks[!color_to_move] & board.get_color_occ(color_to_move) & board.get_piece_occ(piece::KING)) != 0;
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
