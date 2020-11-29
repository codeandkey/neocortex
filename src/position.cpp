/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "position.h"
#include "util.h"
#include "attacks.h"
#include "eval_consts.h"

#include "log.h"

#include <cassert>
#include <climits>

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
	first_state.key = 0;
	first_state.key ^= board.get_tt_key();
	first_state.key ^= zobrist::castle(first_state.castle_rights);
	first_state.in_check = 0;

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

	first_state.key = 0;
	first_state.key ^= board.get_tt_key();
	first_state.key ^= zobrist::en_passant(first_state.en_passant_square);
	first_state.key ^= zobrist::castle(first_state.castle_rights);

	first_state.in_check = test_check(color_to_move);

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
	assert(!test_check(!color_to_move));
	assert(move.is_valid());
	assert(piece::color(board.get_piece(move.src())) == color_to_move);

	State last_state = ply.back();
	ply.push_back(last_state);

	State& next_state = ply.back();

	next_state.last_move = move;
	
	if (color_to_move == piece::BLACK) {
		next_state.fullmove_number++;
	}

	int dst_piece = board.get_piece(move.dst());
	int src_piece = board.remove(move.src());

	next_state.halfmove_clock++;

	if (piece::type(src_piece) == piece::PAWN) {
		next_state.halfmove_clock = 0;
	}

	next_state.en_passant_square = square::null;
	next_state.captured_piece = piece::null;
	next_state.captured_square = square::null;

	/* Check for EP capture */
	if (piece::type(src_piece) == piece::PAWN && move.dst() == last_state.en_passant_square) {
		/* EP, need to remove piece */
		int adv_dir = (color_to_move == piece::WHITE) ? NORTH : SOUTH;
		int capture_square = last_state.en_passant_square - adv_dir;
 		int removed = board.remove(capture_square);

		next_state.captured_piece = removed;
		next_state.captured_square = capture_square;
		next_state.halfmove_clock = 0;

		assert(piece::type(removed) == piece::PAWN);
	}

	/* Move rook also if castling */
	if (piece::type(src_piece) == piece::KING && abs(square::file(move.src()) - square::file(move.dst())) > 1) {
		int is_ks = move.dst() > move.src();
		int castle_rank = (color_to_move == piece::WHITE) ? 0 : 7;
		int rook_file = is_ks ? 7 : 0;
		int rook_dstfile = is_ks ? 5 : 3;

		int rook_src = square::at(castle_rank, rook_file);
		int rook_dst = square::at(castle_rank, rook_dstfile);

		board.place(rook_dst, board.remove(rook_src));
	}

	/* Test if destination is occupied */
	if (piece::is_valid(dst_piece)) {
		/* Normal capture */
		next_state.captured_piece = board.replace(move.dst(), src_piece);
		next_state.captured_square = move.dst();
		next_state.halfmove_clock = 0;

		/* no king captures */
		assert(piece::type(next_state.captured_piece) != piece::KING);
	} else {
		/* Quiet */
		board.place(move.dst(), src_piece);
	}

	/* Apply promotion */
	if (piece::is_type(move.ptype())) {
		board.replace(move.dst(), piece::make_piece(color_to_move, move.ptype()));
	}

	/* Revoke castling rights */
	if (piece::type(src_piece) == piece::KING) {
		int rights = (color_to_move == piece::WHITE) ? 0x3 : 0xC;
		next_state.castle_rights &= ~rights;
	}

	static const bitboard white_ks_revoke = bb::mask(square::at(0, 4)) | bb::mask(square::at(0, 7));
	static const bitboard white_qs_revoke = bb::mask(square::at(0, 4)) | bb::mask(square::at(0, 0));
	static const bitboard black_ks_revoke = bb::mask(square::at(7, 4)) | bb::mask(square::at(7, 7));
	static const bitboard black_qs_revoke = bb::mask(square::at(7, 4)) | bb::mask(square::at(7, 0));

	bitboard src_dst_mask = bb::mask(move.src()) | bb::mask(move.dst());

	if (src_dst_mask & white_ks_revoke) {
		next_state.castle_rights &= ~CASTLE_WHITE_K;
	}

	if (src_dst_mask & white_qs_revoke) {
		next_state.castle_rights &= ~CASTLE_WHITE_Q;
	}

	if (src_dst_mask & black_ks_revoke) {
		next_state.castle_rights &= ~CASTLE_BLACK_K;
	}

	if (src_dst_mask & black_qs_revoke) {
		next_state.castle_rights &= ~CASTLE_BLACK_Q;
	}

	/* Update en passant */
	if (piece::type(src_piece) == piece::PAWN && abs(square::rank(move.dst()) - square::rank(move.src())) > 1) {
		int epsq = move.dst() + ((color_to_move == piece::WHITE) ? SOUTH : NORTH);
		next_state.en_passant_square = epsq;

		if (color_to_move == piece::WHITE) {
			assert(epsq >= 16 && epsq <= 23);
		}
		else {
			assert(epsq >= 40 && epsq <= 47);
		}
	}

	/* Flip color to move */
	color_to_move = !color_to_move;

	/* Update zobrist key */
	next_state.key = 0;
	next_state.key ^= board.get_tt_key();
	next_state.key ^= zobrist::en_passant(next_state.en_passant_square);
	next_state.key ^= zobrist::castle(next_state.castle_rights);

	next_state.in_check = test_check(color_to_move);

	if (color_to_move == piece::BLACK) {
		next_state.key ^= zobrist::black_to_move();
	}

	/* Check that king is not in attack */
	return !test_check(!color_to_move);
}

void Position::unmake_move(Move move) {
	assert(ply.size() > 1);
	assert(ply.back().last_move == move);

	State last_state = ply.back();
	ply.pop_back();

	/* Flip CTM early for readability */
	color_to_move = !color_to_move;

	/* Unpromote */
	if (piece::is_type(move.ptype())) {
		board.replace(move.dst(), piece::make_piece(color_to_move, piece::PAWN));
	}

	/* Move piece back to source */
	int dst_piece = board.remove(move.dst());
	board.place(move.src(), dst_piece);

	/* Unmove rook in castling */
	if (piece::type(dst_piece) == piece::KING && abs(square::file(move.src()) - square::file(move.dst())) > 1) {
		int is_ks = move.dst() > move.src();
		int castle_rank = (color_to_move == piece::WHITE) ? 0 : 7;
		int rook_file = is_ks ? 5 : 3;
		int rook_dstfile = is_ks ? 7 : 0;

		int rook_src = square::at(castle_rank, rook_file);
		int rook_dst = square::at(castle_rank, rook_dstfile);

		board.place(rook_dst, board.remove(rook_src));
	}

	/* Replace captured pieces */
	if (piece::is_valid(last_state.captured_piece)) {
		board.place(last_state.captured_square, last_state.captured_piece);
	}
}

bitboard Position::en_passant_mask() {
	int sq = ply.back().en_passant_square;
	return (sq == square::null) ? 0ULL : bb::mask(sq);
}

int Position::pseudolegal_moves(Move* out) {
	if (check()) {
		return pseudolegal_moves_evasions(out);
	}

	int count = 0;

	bitboard ctm = board.get_color_occ(color_to_move);
	bitboard opp = board.get_color_occ(!color_to_move);
	
	bitboard ep_mask = 0;
	int ep_square = ply.back().en_passant_square;

	if (square::is_valid(ep_square)) {
		ep_mask = bb::mask(ep_square);
	}

	/* Pawn moves */
	bitboard pawns = ctm & board.get_piece_occ(piece::PAWN);

	bitboard promoting_rank = (color_to_move == piece::WHITE) ? RANK_7 : RANK_2;
	bitboard starting_rank = (color_to_move == piece::WHITE) ? RANK_2 : RANK_7;

	int adv_dir = (color_to_move == piece::WHITE) ? NORTH : SOUTH;
	int left_dir = (color_to_move == piece::WHITE) ? NORTHWEST : SOUTHWEST;
	int right_dir = (color_to_move == piece::WHITE) ? NORTHEAST : SOUTHEAST;

	bitboard promoting_pawns = pawns & promoting_rank;

	/* Promoting left captures */
	bitboard promoting_left_cap = bb::shift(promoting_pawns & ~FILE_A, left_dir) & opp;

	while (promoting_left_cap) {
		int dst = bb::poplsb(promoting_left_cap);
		out[count++] = Move(dst - left_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - left_dir, dst, piece::KNIGHT);
		out[count++] = Move(dst - left_dir, dst, piece::ROOK);
		out[count++] = Move(dst - left_dir, dst, piece::BISHOP);
	}

	/* Promoting right captures */
	bitboard promoting_right_cap = bb::shift(promoting_pawns & ~FILE_H, right_dir) & opp;

	while (promoting_right_cap) {
		int dst = bb::poplsb(promoting_right_cap);
		out[count++] = Move(dst - right_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - right_dir, dst, piece::KNIGHT);
		out[count++] = Move(dst - right_dir, dst, piece::ROOK);
		out[count++] = Move(dst - right_dir, dst, piece::BISHOP);
	}

	/* Promoting advances */
	bitboard promoting_advances = bb::shift(promoting_pawns, adv_dir) & ~board.get_global_occ();

	while (promoting_advances) {
		int dst = bb::poplsb(promoting_advances);
		out[count++] = Move(dst - adv_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - adv_dir, dst, piece::KNIGHT);
		out[count++] = Move(dst - adv_dir, dst, piece::ROOK);
		out[count++] = Move(dst - adv_dir, dst, piece::BISHOP);
	}

	/* Nonpromoting pawn moves */
	bitboard npm_pawns = pawns & ~promoting_pawns;

	/* Advances */
	bitboard npm_advances = bb::shift(npm_pawns, adv_dir) & ~board.get_global_occ();

	while (npm_advances) {
		int dst = bb::poplsb(npm_advances);
		out[count++] = Move(dst - adv_dir, dst);
	}

	/* Left captures */
	bitboard npm_left_cap = bb::shift(npm_pawns & ~FILE_A, left_dir) & (opp | ep_mask);

	while (npm_left_cap) {
		int dst = bb::poplsb(npm_left_cap);
		out[count++] = Move(dst - left_dir, dst);
	}

	/* Right captures */
	bitboard npm_right_cap = bb::shift(npm_pawns & ~FILE_H, right_dir) & (opp | ep_mask);

	while (npm_right_cap) {
		int dst = bb::poplsb(npm_right_cap);
		out[count++] = Move(dst - right_dir, dst);
	}

	/* Jumps */
	bitboard npm_jumps = bb::shift(pawns & starting_rank, adv_dir) & ~board.get_global_occ();
	npm_jumps = bb::shift(npm_jumps, adv_dir) & ~board.get_global_occ();

	while (npm_jumps) {
		int dst = bb::poplsb(npm_jumps);
		out[count++] = Move(dst - 2 * adv_dir, dst);
	}

	/* Queen moves */
	bitboard queens = ctm & board.get_piece_occ(piece::QUEEN);
	
	while (queens) {
		int src = bb::poplsb(queens);
		bitboard moves = attacks::queen(src, board.get_global_occ()) & ~ctm;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Rook moves */
	bitboard rooks = ctm & board.get_piece_occ(piece::ROOK);

	while (rooks) {
		int src = bb::poplsb(rooks);
		bitboard moves = attacks::rook(src, board.get_global_occ()) & ~ctm;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Knight moves */
	bitboard knights = ctm & board.get_piece_occ(piece::KNIGHT);

	while (knights) {
		int src = bb::poplsb(knights);
		bitboard moves = attacks::knight(src) & ~ctm;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Bishop moves */
	bitboard bishops = ctm & board.get_piece_occ(piece::BISHOP);

	while (bishops) {
		int src = bb::poplsb(bishops);
		bitboard moves = attacks::bishop(src, board.get_global_occ()) & ~ctm;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* King moves */
	bitboard kings = ctm & board.get_piece_occ(piece::KING);

	while (kings) {
		int src = bb::poplsb(kings);
		bitboard moves = attacks::king(src) & ~ctm;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Castling moves */

	/* We perform most of the legal move tests here, notably the noattack and occ tests */
	bitboard castle_rank = (color_to_move == piece::WHITE) ? RANK_1 : RANK_8;
	bitboard noattack_ks = castle_rank & (FILE_E | FILE_F | FILE_G);
	bitboard noattack_qs = castle_rank & (FILE_E | FILE_D | FILE_C);
	bitboard no_occ_ks = castle_rank & (FILE_F | FILE_G);
	bitboard no_occ_qs = castle_rank & (FILE_B | FILE_C | FILE_D);

	int king_src = (color_to_move == piece::WHITE) ? square::at(0, 4) : square::at(7, 4);
	int ks_dst = (color_to_move == piece::WHITE) ? square::at(0, 6) : square::at(7, 6);
	int qs_dst = (color_to_move == piece::WHITE) ? square::at(0, 2) : square::at(7, 2);

	/* Kingside */
	if (ply.back().castle_rights & (1 << (color_to_move * 2))) {
		/* occ test */
		if (!(board.get_global_occ() & no_occ_ks)) {
			/* noattack test */
			if (!board.mask_is_attacked(noattack_ks, !color_to_move)) {
				out[count++] = Move(king_src, ks_dst);
			}
		}
	}

	/* Queenside */
	if (ply.back().castle_rights & (1 << (color_to_move * 2 + 1))) {
		/* occ test */
		if (!(board.get_global_occ() & no_occ_qs)) {
			/* noattack test */
			if (!board.mask_is_attacked(noattack_qs, !color_to_move)) {
				out[count++] = Move(king_src, qs_dst);
			}
		}
	}

	assert(count <= MAX_PL_MOVES);
	return count;
}

int Position::pseudolegal_moves_evasions(Move* out) {
	int count = 0;

	assert(check());

	bitboard ctm = board.get_color_occ(color_to_move);
	bitboard opp = board.get_color_occ(!color_to_move);

	/* King moves */
	bitboard kings = ctm & board.get_piece_occ(piece::KING);

	/* Also grab attackers */
	int king_sq = bb::getlsb(kings);
	bitboard attackers = board.attacks_on(king_sq) & opp;

	while (kings) {
		int src = bb::poplsb(kings);
		bitboard moves = attacks::king(src) & ~ctm;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	if (bb::popcount(attackers) > 1) {
		/* double check, king moves only */
		return count;
	}

	/* Only one attacker -- look for pieces that can capture it */
	int attacker_square = bb::getlsb(attackers);
	bitboard capture_evade = board.attacks_on(attacker_square) & ctm;

	/* If checker is sliding piece, add blocking moves */
	bitboard block_dsts = bb::between(king_sq, attacker_square);

	bitboard ep_mask = 0;
	int ep_square = ply.back().en_passant_square;

	if (square::is_valid(ep_square)) {
		/* Only consider ep captures if they can block the check */
		ep_mask = bb::mask(ep_square) & block_dsts;
	}

	/* Pawn moves */
	bitboard pawns = ctm & board.get_piece_occ(piece::PAWN);

	bitboard promoting_rank = (color_to_move == piece::WHITE) ? RANK_7 : RANK_2;
	bitboard starting_rank = (color_to_move == piece::WHITE) ? RANK_2 : RANK_7;

	int adv_dir = (color_to_move == piece::WHITE) ? NORTH : SOUTH;
	int left_dir = (color_to_move == piece::WHITE) ? NORTHWEST : SOUTHWEST;
	int right_dir = (color_to_move == piece::WHITE) ? NORTHEAST : SOUTHEAST;

	bitboard promoting_pawns = pawns & promoting_rank;

	/* Promoting left captures */
	bitboard promoting_left_cap = bb::shift(promoting_pawns & ~FILE_A, left_dir) & capture_evade;

	while (promoting_left_cap) {
		int dst = bb::poplsb(promoting_left_cap);
		out[count++] = Move(dst - left_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - left_dir, dst, piece::KNIGHT);
		out[count++] = Move(dst - left_dir, dst, piece::ROOK);
		out[count++] = Move(dst - left_dir, dst, piece::BISHOP);
	}

	/* Promoting right captures */
	bitboard promoting_right_cap = bb::shift(promoting_pawns & ~FILE_H, right_dir) & capture_evade;

	while (promoting_right_cap) {
		int dst = bb::poplsb(promoting_right_cap);
		out[count++] = Move(dst - right_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - right_dir, dst, piece::KNIGHT);
		out[count++] = Move(dst - right_dir, dst, piece::ROOK);
		out[count++] = Move(dst - right_dir, dst, piece::BISHOP);
	}

	/* Promoting advances, blocking */
	bitboard promoting_advances = bb::shift(promoting_pawns, adv_dir) & ~board.get_global_occ() & block_dsts;

	while (promoting_advances) {
		int dst = bb::poplsb(promoting_advances);
		out[count++] = Move(dst - adv_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - adv_dir, dst, piece::KNIGHT);
		out[count++] = Move(dst - adv_dir, dst, piece::ROOK);
		out[count++] = Move(dst - adv_dir, dst, piece::BISHOP);
	}

	/* Nonpromoting pawn moves */
	bitboard npm_pawns = pawns & ~promoting_pawns;

	/* Blocking advances */
	bitboard npm_advances = bb::shift(npm_pawns, adv_dir) & ~board.get_global_occ() & block_dsts;

	while (npm_advances) {
		int dst = bb::poplsb(npm_advances);
		out[count++] = Move(dst - adv_dir, dst);
	}

	/* Left captures */
	bitboard npm_left_cap = bb::shift(npm_pawns & ~FILE_A, left_dir) & (attackers | ep_mask);

	while (npm_left_cap) {
		int dst = bb::poplsb(npm_left_cap);
		out[count++] = Move(dst - left_dir, dst);
	}

	/* Right captures */
	bitboard npm_right_cap = bb::shift(npm_pawns & ~FILE_H, right_dir) & (attackers | ep_mask);

	while (npm_right_cap) {
		int dst = bb::poplsb(npm_right_cap);
		out[count++] = Move(dst - right_dir, dst);
	}

	/* Jumps */
	bitboard npm_jumps = bb::shift(pawns & starting_rank, adv_dir) & ~board.get_global_occ();
	npm_jumps = bb::shift(npm_jumps, adv_dir) & ~board.get_global_occ() & block_dsts;

	while (npm_jumps) {
		int dst = bb::poplsb(npm_jumps);
		out[count++] = Move(dst - 2 * adv_dir, dst);
	}

	/* Queen moves */
	bitboard queens = ctm & board.get_piece_occ(piece::QUEEN);

	while (queens) {
		int src = bb::poplsb(queens);
		bitboard moves = attacks::queen(src, board.get_global_occ()) & (block_dsts | attackers);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Rook moves */
	bitboard rooks = ctm & board.get_piece_occ(piece::ROOK);

	while (rooks) {
		int src = bb::poplsb(rooks);
		bitboard moves = attacks::rook(src, board.get_global_occ()) & (block_dsts | attackers);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Knight moves */
	bitboard knights = ctm & board.get_piece_occ(piece::KNIGHT);

	while (knights) {
		int src = bb::poplsb(knights);
		bitboard moves = attacks::knight(src) & (block_dsts | attackers);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Bishop moves */
	bitboard bishops = ctm & board.get_piece_occ(piece::BISHOP);

	while (bishops) {
		int src = bb::poplsb(bishops);
		bitboard moves = attacks::bishop(src, board.get_global_occ()) & (block_dsts | attackers);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	assert(count <= MAX_PL_MOVES);
	return count;
}

int Position::pseudolegal_moves_quiescence(Move* out) {
	if (check()) {
		return pseudolegal_moves_evasions(out);
	}

	int count = 0;

	bitboard ctm = board.get_color_occ(color_to_move);
	bitboard opp = board.get_color_occ(!color_to_move);
	
	int oppking = bb::getlsb(opp & board.get_piece_occ(piece::KING));

	bitboard bchecks = attacks::bishop(oppking, board.get_global_occ()) & ~ctm;
	bitboard rchecks = attacks::rook(oppking, board.get_global_occ()) & ~ctm;
	bitboard nchecks = attacks::knight(oppking) & ~ctm;
	bitboard pchecks = attacks::pawn(!color_to_move, oppking);

	bitboard ep_mask = 0;
	int ep_square = ply.back().en_passant_square;

	if (square::is_valid(ep_square)) {
		ep_mask = bb::mask(ep_square);
	}

	/* Pawn moves */
	bitboard pawns = ctm & board.get_piece_occ(piece::PAWN);
	bitboard promoting_rank = (color_to_move == piece::WHITE) ? RANK_7 : RANK_2;

	int adv_dir = (color_to_move == piece::WHITE) ? NORTH : SOUTH;
	int left_dir = (color_to_move == piece::WHITE) ? NORTHWEST : SOUTHWEST;
	int right_dir = (color_to_move == piece::WHITE) ? NORTHEAST : SOUTHEAST;

	bitboard promoting_pawns = pawns & promoting_rank;

	/* Promoting left captures */
	bitboard promoting_left_cap = bb::shift(promoting_pawns & ~FILE_A, left_dir) & opp;

	while (promoting_left_cap) {
		int dst = bb::poplsb(promoting_left_cap);
		out[count++] = Move(dst - left_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - left_dir, dst, piece::KNIGHT);
	}

	/* Promoting right captures */
	bitboard promoting_right_cap = bb::shift(promoting_pawns & ~FILE_H, right_dir) & opp;

	while (promoting_right_cap) {
		int dst = bb::poplsb(promoting_right_cap);
		out[count++] = Move(dst - right_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - right_dir, dst, piece::KNIGHT);
	}

	/* Promoting advances */
	bitboard promoting_advances = bb::shift(promoting_pawns, adv_dir) & ~board.get_global_occ();

	while (promoting_advances) {
		int dst = bb::poplsb(promoting_advances);
		out[count++] = Move(dst - adv_dir, dst, piece::QUEEN);
		out[count++] = Move(dst - adv_dir, dst, piece::KNIGHT);
	}

	/* Nonpromoting pawn moves */
	bitboard npm_pawns = pawns & ~promoting_pawns;

	/* Left captures */
	bitboard npm_left_cap = bb::shift(npm_pawns & ~FILE_A, left_dir) & (opp | ep_mask);

	while (npm_left_cap) {
		int dst = bb::poplsb(npm_left_cap);
		out[count++] = Move(dst - left_dir, dst);
	}

	/* Right captures */
	bitboard npm_right_cap = bb::shift(npm_pawns & ~FILE_H, right_dir) & (opp | ep_mask);

	while (npm_right_cap) {
		int dst = bb::poplsb(npm_right_cap);
		out[count++] = Move(dst - right_dir, dst);
	}

	/* Pawn checks */
	bitboard npm_checks = bb::shift(npm_pawns, adv_dir) & ~board.get_global_occ() & pchecks;

	while (npm_checks) {
		int dst = bb::poplsb(npm_checks);
		out[count++] = Move(dst - adv_dir, dst);
	}

	/* Queen captures / checks */
	bitboard queens = ctm & board.get_piece_occ(piece::QUEEN);

	while (queens) {
		int src = bb::poplsb(queens);
		bitboard moves = attacks::queen(src, board.get_global_occ()) & (opp | bchecks | rchecks);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Rook captures / checks */
	bitboard rooks = ctm & board.get_piece_occ(piece::ROOK);

	while (rooks) {
		int src = bb::poplsb(rooks);
		bitboard moves = attacks::rook(src, board.get_global_occ()) & (opp | rchecks);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Knight captures / checks */
	bitboard knights = ctm & board.get_piece_occ(piece::KNIGHT);

	while (knights) {
		int src = bb::poplsb(knights);
		bitboard moves = attacks::knight(src) & (opp | nchecks);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* Bishop captures / checks */
	bitboard bishops = ctm & board.get_piece_occ(piece::BISHOP);

	while (bishops) {
		int src = bb::poplsb(bishops);
		bitboard moves = attacks::bishop(src, board.get_global_occ()) & (opp | bchecks);

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	/* King captures */
	bitboard kings = ctm & board.get_piece_occ(piece::KING);

	while (kings) {
		int src = bb::poplsb(kings);
		bitboard moves = attacks::king(src) & opp;

		while (moves) {
			int dst = bb::poplsb(moves);
			out[count++] = Move(src, dst);
		}
	}

	assert(count <= MAX_PL_MOVES);
	return count;
}

void Position::order_moves(Move* moves, int num_moves, Move pv_move) {
	int scores[MAX_PL_MOVES] = { 0 };

	for (int i = 0; i < num_moves; ++i) {
		/* Assign move scores */

		// PV move bonus
		if (moves[i] == pv_move) scores[i] += eval::ORDER_PV_MOVE;

		// SEE on basic captures
		if (board.get_color_occ(!color_to_move) & bb::mask(moves[i].dst())) scores[i] += see_capture(moves[i]);

		// SEE on EP captures
		if (piece::type(board.get_piece(moves[i].src())) == piece::PAWN && moves[i].dst() == ply.back().en_passant_square) scores[i] += see_capture(moves[i]);
	}

	/* Perform selection sort */
	for (int i = 0; i < num_moves - 1; ++i) {
		int m = scores[i];
		int sind = i;

		for (int j = i + 1; j < num_moves; ++j) {
			if (scores[j] > m) {
				sind = j;
				m = scores[j];
			}
		}

		if (sind != i) {
			int tmp = scores[sind];
			scores[sind] = scores[i];
			scores[i] = tmp;

			Move tmove = moves[sind];
			moves[sind] = moves[i];
			moves[i] = tmove;
		}
	}
}

int Position::order_moves_quiescence(Move* moves, int num_moves, Move pv_move) {
	int scores[MAX_PL_MOVES] = { 0 };
	int new_num_moves = num_moves;

	for (int i = 0; i < num_moves; ++i) {
		/* Assign move scores */

		// PV move bonus
		if (moves[i] == pv_move) scores[i] += eval::ORDER_PV_MOVE;

		if (
			(board.get_color_occ(!color_to_move) & bb::mask(moves[i].dst()) & ~board.get_piece_occ(piece::KING)) || // Normal captures
			(piece::type(board.get_piece(moves[i].src())) == piece::PAWN && moves[i].dst() == ply.back().en_passant_square) // EP captures
			) {
			int see_val = see_capture(moves[i]);

			if (see_val < 0) {
				moves[i] = Move::null; /* Drop moves with negative SEE */
				scores[i] = INT_MIN;
				--new_num_moves;
			}
			else {
				scores[i] += see_capture(moves[i]);
			}
		}
	}

	/* Perform selection sort */
	for (int i = 0; i < new_num_moves; ++i) {
		int m = scores[i];
		int sind = i;

		for (int j = i + 1; j < num_moves; ++j) {
			if (!moves[j].is_valid()) continue;
			if (scores[j] > m) {
				sind = j;
				m = scores[j];
			}
		}

		if (sind != i) {
			int tmp = scores[sind];
			scores[sind] = scores[i];
			scores[i] = tmp;

			Move tmove = moves[sind];
			moves[sind] = moves[i];
			moves[i] = tmove;
		}
	}

	return new_num_moves;
}

int Position::see_capture(Move cap) {
	assert(!(bb::mask(cap.dst()) & board.get_piece_occ(piece::KING)));

	bool ep = false;
	int csq = cap.dst();

	if (piece::type(board.get_piece(cap.src())) == piece::PAWN && bb::mask(cap.dst()) == en_passant_mask()) {
		ep = true;
		csq = cap.dst() + ((color_to_move == piece::WHITE) ? SOUTH : NORTH);
	}

	int capped = piece::null;

	/* Make capture */
	if (ep) {
		capped = board.remove(csq);
		board.place(cap.dst(), board.remove(cap.src()));
	} else {
		capped = board.replace(cap.dst(), board.remove(cap.src()));
	}

	int ret = eval::MATERIAL_MG[piece::type(capped)] - see(cap.dst(), !color_to_move);

	/* Undo capture */
	if (ep) {
		board.place(csq, capped);
		board.place(cap.src(), board.remove(cap.dst()));
	}
	else {
		board.place(cap.src(), board.replace(cap.dst(), capped));
	}

	return ret;
}

int Position::see(int sq, int attacking_col) {
	bitboard ctm = board.get_color_occ(attacking_col);
	int lva = square::null;

#ifndef NDEBUG
	zobrist::Key starting_bkey = board.get_tt_key();
#endif

	/* Find least valuable attacker */

	// Search for pawn attacks
	bitboard pawn_attackers = attacks::pawn(!attacking_col, sq) & board.get_piece_occ(piece::PAWN) & ctm;

	if (pawn_attackers) {
		lva = bb::poplsb(pawn_attackers);
	}
	else {
		// Find bishop attackers
		bitboard bishop_attacks = attacks::bishop(sq, board.get_global_occ());
		bitboard bishop_attackers = bishop_attacks & ctm & board.get_piece_occ(piece::BISHOP);

		if (bishop_attackers) {
			lva = bb::poplsb(bishop_attackers);
		}
		else {
			// Find knight attackers
			bitboard knight_attackers = attacks::knight(sq) & ctm & board.get_piece_occ(piece::KNIGHT);

			if (knight_attackers) {
				lva = bb::poplsb(knight_attackers);
			}
			else {
				// Find rook attackers
				bitboard rook_attacks = attacks::rook(sq, board.get_global_occ());
				bitboard rook_attackers = rook_attacks & ctm & board.get_piece_occ(piece::ROOK);

				if (rook_attackers) {
					lva = bb::poplsb(rook_attackers);
				}
				else {
					// Find queen attackers
					bitboard queen_attackers = (bishop_attacks | rook_attacks) & ctm & board.get_piece_occ(piece::QUEEN);

					if (queen_attackers) {
						lva = bb::poplsb(queen_attackers);
					}
					else {
						// Find king attackers
						bitboard king_attackers = attacks::king(sq) & ctm & board.get_piece_occ(piece::KING);

						if (king_attackers) {
							lva = bb::poplsb(king_attackers);
						}
					}
				}
			}
		}
	}

	if (square::is_valid(lva)) {
		/* Remove LVA from board */
		int removed = board.remove(lva);

		/* Perform capture with LVA, record material gain */
		int dst_removed = board.replace(sq, removed);
		int dst_value = eval::MATERIAL_MG[piece::type(dst_removed)];

		int ret = dst_value - see(sq, !attacking_col);

		/* Undo capture */
		board.replace(sq, dst_removed);
		board.place(lva, removed);

		assert(board.get_tt_key() == starting_bkey);
		return ret;
	} else {
		// No valid attackers, SEE is 0
		assert(board.get_tt_key() == starting_bkey);
		return 0;
	}
}

int Position::evaluate(std::string* dbg) {
	int phase;
	int center_control;
	int king_safety;
	int passed_pawns;
	int passed_pawn_adv;

	/* Start evaluation */
	int score = 0;

	/* Compute game phase */
	phase = eval::PHASE_TOTAL;

	for (int t = 0; t < 5; ++t) {
		phase -= bb::popcount(board.get_piece_occ(t)) * eval::PHASE_VALS[t];
	}

	phase = (phase * 256) / eval::PHASE_TOTAL;

	/* Compute material values */
	score += (board.material_mg() * (256 - phase)) / 256;
	score += (board.material_eg() * phase) / 256;

	/* Compute center square control */
	static const int center_squares[4] = { 27, 28, 35, 36 };
	center_control = 0;

	for (int i = 0; i < 4; ++i) {
		center_control += board.guard_value(center_squares[i]) * eval::CENTER_CONTROL;
	}

	/* Compute king safety */
	king_safety = 0;

	bitboard white_ksq = attacks::king(bb::getlsb(board.get_color_occ(piece::WHITE) & board.get_piece_occ(piece::KING)));
	bitboard black_ksq = attacks::king(bb::getlsb(board.get_color_occ(piece::BLACK) & board.get_piece_occ(piece::KING)));

	while (white_ksq) {
		int ksq = bb::poplsb(white_ksq);
		int gv = board.guard_value(ksq);

		/* don't incentivize overprotecting the king */
		if (gv > 0) {
			gv = 0;
		}

		king_safety += gv;
	}

	while (black_ksq) {
		int ksq = bb::poplsb(black_ksq);
		int gv = board.guard_value(ksq);

		/* don't incentivize overprotecting the king */
		if (gv < 0) {
			gv = 0;
		}

		king_safety += gv;
	}

	score += king_safety * eval::KING_SAFETY;

	/* Find passed pawns */
	bitboard passers[2];

	passers[piece::WHITE] = board.passedpawns(piece::WHITE);
	passers[piece::BLACK] = board.passedpawns(piece::BLACK);

	/* Apply passed pawn bonus */
	passed_pawns = 0;

	passed_pawns += bb::popcount(passers[piece::WHITE]) * eval::PASSED_PAWNS;
	passed_pawns -= bb::popcount(passers[piece::BLACK]) * eval::PASSED_PAWNS;

	score += passed_pawns;

	/* Apply advance bonus for passed pawns */
	bitboard tmp_passers;
	passed_pawn_adv = 0;

	tmp_passers = passers[piece::WHITE];
	while (tmp_passers) {
		int p = bb::poplsb(tmp_passers);
		passed_pawn_adv += square::rank(p);
	}

	tmp_passers = passers[piece::BLACK];
	while (tmp_passers) {
		int p = bb::poplsb(tmp_passers);
		passed_pawn_adv -= (7 - square::rank(p));
	}

	score += passed_pawn_adv * eval::ADV_PASSEDPAWN;

	/* Write debug if needed */
	if (dbg) {
		std::string output;

		output += "            | white | black |\n";
		output += "------------|-------|-------|\n";
		output += util::format("material_mg | %13d |\n", board.material_mg());
		output += util::format("material_eg | %13d |\n", board.material_eg());
		output += util::format("ctr_control | %13d |\n", center_control);
		output += util::format("king_safety | %13d |\n", king_safety);
		output += util::format("adv_passed  | %13d |\n", passed_pawn_adv);
		output += util::format("passed_pns  | %13d |\n", passed_pawns);
		output += util::format("phase       | %13d |\n", phase);

		*dbg = output;
	}

	/* Flip eval if BTM */
	if (color_to_move == piece::BLACK) {
		score *= -1;
	}

	return score + eval::TEMPO_BONUS;
}

zobrist::Key Position::get_tt_key() {
	return ply.back().key;
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
