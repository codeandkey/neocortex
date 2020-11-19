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
	next_state.key ^= zobrist::castle(castle_rights());

	if (color_to_move == piece::BLACK) {
		next_state.key ^= zobrist::black_to_move();
	}

	/* Check that king is not in attack */
	return !check(!color_to_move);
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
	bitboard npm_pawns = pawns ^ promoting_pawns;

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

int Position::see_capture(Move cap) {
	int ret = SEE_ILLEGAL;

	if (make_move(cap)) {
		ret = see(cap.dst());
	}

	unmake_move(cap);
	return ret;
}

int Position::see(int sq, bitboard valid_attackers) {
	bitboard mask = bb::mask(sq);

	bitboard ctm = board.get_color_occ(color_to_move);
	int value = 0;
	int lva = square::null;

	zobrist::Key starting_bkey = board.get_tt_key();

	/* Find least valuable attacker */

	// Search for pawn attacks
	bitboard pawn_attackers = attacks::pawn(!color_to_move, sq) & board.get_piece_occ(piece::PAWN) & ctm & valid_attackers;

	if (pawn_attackers) {
		lva = bb::poplsb(pawn_attackers);
	}
	else {
		// Find bishop attackers
		bitboard bishop_attacks = attacks::bishop(sq, board.get_global_occ());
		bitboard bishop_attackers = bishop_attacks & ctm & board.get_piece_occ(piece::BISHOP) & valid_attackers;

		if (bishop_attackers) {
			lva = bb::poplsb(bishop_attackers);
		}
		else {
			// Find knight attackers
			bitboard knight_attackers = attacks::knight(sq) & ctm & board.get_piece_occ(piece::KNIGHT) & valid_attackers;

			if (knight_attackers) {
				lva = bb::poplsb(knight_attackers);
			}
			else {
				// Find rook attackers
				bitboard rook_attacks = attacks::rook(sq, board.get_global_occ());
				bitboard rook_attackers = rook_attacks & ctm & board.get_piece_occ(piece::ROOK) & valid_attackers;

				if (rook_attackers) {
					lva = bb::poplsb(rook_attackers);
				}
				else {
					// Find queen attackers
					bitboard queen_attackers = (bishop_attacks | rook_attacks) & ctm & board.get_piece_occ(piece::QUEEN) & valid_attackers;

					if (queen_attackers) {
						lva = bb::poplsb(queen_attackers);
					}
					else {
						// Find king attackers
						bitboard king_attackers = attacks::king(sq) & ctm & board.get_piece_occ(piece::KING) & valid_attackers;

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

		color_to_move = !color_to_move;

		int ret = dst_value - see(sq);

		/* Undo capture, reset color to move */
		color_to_move = !color_to_move;

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

	int material_mg[2];
	int material_eg[2];
	int attackbonus[2];
	int mobility_mg[2], mobility_eg[2];
	int center_control[2];
	int king_safety[2];
	int blocking_pawns[2];
	int passed_pawns[2];
	int adv_pawn_mg[2], adv_pawn_eg[2];
	int adv_passedpawn_mg[2], adv_passedpawn_eg[2];
	int king_adv_mg[2], king_adv_eg[2];

	/* Compute material values */
	material_mg[piece::WHITE] = material_mg[piece::BLACK] = 0;
	material_eg[piece::WHITE] = material_eg[piece::BLACK] = 0;

	for (int t = 0; t < 6; ++t) {
		int white = bb::popcount(board.get_color_occ(piece::WHITE) & board.get_piece_occ(t));
		int black = bb::popcount(board.get_color_occ(piece::BLACK) & board.get_piece_occ(t));

		material_mg[piece::WHITE] += white * eval::MATERIAL_MG[t];
		material_mg[piece::BLACK] += black * eval::MATERIAL_MG[t];
		material_eg[piece::WHITE] += white * eval::MATERIAL_EG[t];
		material_eg[piece::BLACK] += black * eval::MATERIAL_EG[t];
	}

	phase = ((material_mg[piece::WHITE] + material_mg[piece::BLACK]) * 256) / eval::MATERIAL_MG_MAX;

	/* Compute attack mobility bonus */
	attackbonus[piece::WHITE] = 0;// bb::popcount(board.attacks(piece::WHITE))* eval::ATTACK_BONUS;
	attackbonus[piece::BLACK] = 0;// bb::popcount(board.attacks(piece::BLACK))* eval::ATTACK_BONUS;

	/* Compute safe mobility bonus */
	for (int c = 0; c < 2; ++c) {
		mobility_mg[c] = mobility_eg[c] = 0;
		continue;

		/*int bishop_mobility = 0, knight_mobility = 0, rook_mobility = 0, queen_mobility = 0;

		bitboard bishops = board.get_color_occ(c) & board.get_piece_occ(piece::BISHOP);

		while (bishops) {
			bishop_mobility += bb::popcount(~board.attacks(!c) & attacks::bishop(bb::poplsb(bishops), board.get_global_occ()));
		}

		bitboard knights = board.get_color_occ(c) & board.get_piece_occ(piece::KNIGHT);

		while (knights) {
			knight_mobility += bb::popcount(~board.attacks(!c) & attacks::knight(bb::poplsb(knights)));
		}

		bitboard rooks = board.get_color_occ(c) & board.get_piece_occ(piece::ROOK);

		while (rooks) {
			rook_mobility += bb::popcount(~board.attacks(!c) & attacks::rook(bb::poplsb(rooks), board.get_global_occ()));
		}

		bitboard queens = board.get_color_occ(c) & board.get_piece_occ(piece::QUEEN);

		while (queens) {
			queen_mobility += bb::popcount(~board.attacks(!c) & attacks::queen(bb::poplsb(queens), board.get_global_occ()));
		}

		mobility_mg[c] += bishop_mobility * eval::MOBILITY_BISHOP;
		mobility_mg[c] += knight_mobility * eval::MOBILITY_KNIGHT;
		mobility_mg[c] += rook_mobility * eval::MOBILITY_ROOK_MG;
		mobility_mg[c] += queen_mobility * eval::MOBILITY_QUEEN;

		mobility_eg[c] += bishop_mobility * eval::MOBILITY_BISHOP;
		mobility_eg[c] += knight_mobility * eval::MOBILITY_KNIGHT;
		mobility_eg[c] += rook_mobility * eval::MOBILITY_ROOK_EG;
		mobility_eg[c] += queen_mobility * eval::MOBILITY_QUEEN;*/
	}

	center_control[piece::WHITE] = center_control[piece::BLACK] = 0;

	/* Compute center square control */
	int center_squares[4] = { 27, 28, 35, 36 };

	for (int i = 0; i < 4; ++i) {
		//center_control[piece::WHITE] += board.get_ad(piece::WHITE)[center_squares[i]] * eval::CENTER_CONTROL;
		//center_control[piece::BLACK] += board.get_ad(piece::BLACK)[center_squares[i]] * eval::CENTER_CONTROL;
	}

	/* Compute king safety */
	king_safety[piece::WHITE] = king_safety[piece::BLACK] = 0;
	king_adv_mg[piece::WHITE] = king_adv_mg[piece::BLACK] = 0;
	king_adv_eg[piece::WHITE] = king_adv_eg[piece::BLACK] = 0;

	for (int c = 0; c < 2; ++c) {
		int ksq = bb::getlsb(board.get_color_occ(c) & board.get_piece_occ(piece::KING));
		bitboard king_squares = attacks::king(ksq);

		while (king_squares) {
			int next = bb::poplsb(king_squares);

			/* this could be better, but could also incentivise severely overprotecting the king */
			//king_safety[c] += (board.get_ad(c)[next] - board.get_ad(!c)[next]) * eval::KING_SAFETY;
		}

		int adv = square::rank(ksq);

		if (c == piece::BLACK) {
			adv = 7 - adv;
		}

		king_adv_mg[c] += adv * eval::KING_ADV_MG;
		king_adv_eg[c] += adv * eval::KING_ADV_EG;
	}

	/* Compute blocking pawn penalties, passed pawn bonus */
	blocking_pawns[piece::WHITE] = blocking_pawns[piece::BLACK] = 0;
	passed_pawns[piece::WHITE] = passed_pawns[piece::BLACK] = 0;
	adv_pawn_mg[piece::WHITE] = adv_pawn_mg[piece::BLACK] = 0;
	adv_pawn_eg[piece::WHITE] = adv_pawn_eg[piece::BLACK] = 0;
	adv_passedpawn_mg[piece::WHITE] = adv_passedpawn_mg[piece::BLACK] = 0;
	adv_passedpawn_eg[piece::WHITE] = adv_passedpawn_eg[piece::BLACK] = 0;
	passed_pawns[piece::WHITE] = passed_pawns[piece::BLACK] = 0;

	for (int c = 0; c < 2; ++c) {
		for (int f = 0; f < 8; ++f) {
			bitboard pawns = bb::file(f) & board.get_color_occ(c) & board.get_piece_occ(piece::PAWN);
			int count = bb::popcount(pawns);

			if (count) {
				if (count > 1) {
					blocking_pawns[c] += eval::BLOCKING_PAWNS;
				}

				bool passed = !bb::popcount(bb::file(f) & board.get_color_occ(!c) & board.get_piece_occ(piece::PAWN));

				if (passed) {
					passed_pawns[c] += eval::PASSED_PAWNS * count;
				}

				/* Grab pawn ranks and apply bonuses */
				while (pawns) {
					int adv = square::rank(bb::poplsb(pawns));

					if (c == piece::BLACK) {
						adv = 7 - adv;
					}

					adv_pawn_mg[c] += eval::ADV_PAWN_MG * adv;
					adv_pawn_eg[c] += eval::ADV_PAWN_EG * adv;

					if (passed) {
						adv_passedpawn_mg[c] += eval::ADV_PASSEDPAWN_MG * adv;
						adv_passedpawn_eg[c] += eval::ADV_PASSEDPAWN_EG * adv;
					}
				}
			}
		}
	}

	/* Compute score */
	int score = 0;
	int ctm = color_to_move;
	int opp = !ctm;

	int material_score_mg = material_mg[ctm] - material_mg[opp];
	int material_score_eg = material_eg[ctm] - material_eg[opp];

	score += (material_score_mg * phase) / 256;
	score += (material_score_eg * (256 - phase)) / 256;

	score += attackbonus[ctm] - attackbonus[opp];

	int mobility_score_mg = mobility_mg[ctm] - mobility_mg[opp];
	int mobility_score_eg = mobility_eg[ctm] - mobility_eg[opp];

	score += (mobility_score_mg * phase) / 256;
	score += (mobility_score_eg * (256 - phase)) / 256;

	score += (phase * (center_control[ctm] - center_control[opp])) / 256;

	score += king_safety[ctm] - king_safety[opp];

	score += blocking_pawns[ctm] - blocking_pawns[opp];

	int adv_pawn_score_mg = adv_pawn_mg[ctm] - adv_pawn_mg[opp];
	int adv_pawn_score_eg = adv_pawn_eg[ctm] - adv_pawn_eg[opp];
	int adv_passedpawn_score_mg = adv_passedpawn_mg[ctm] - adv_passedpawn_mg[opp];
	int adv_passedpawn_score_eg = adv_passedpawn_eg[ctm] - adv_passedpawn_eg[opp];

	int adv_king_score_mg = king_adv_mg[ctm] - king_adv_mg[opp];
	int adv_king_score_eg = king_adv_eg[ctm] - king_adv_eg[opp];

	score += (phase * adv_pawn_score_mg) / 256;
	score += ((256 - phase) * adv_pawn_score_eg) / 256;
	score += (phase * adv_passedpawn_score_mg) / 256;
	score += ((256 - phase) * adv_passedpawn_score_eg) / 256;

	score += (phase * adv_king_score_mg) / 256;
	score += ((256 - phase) * adv_king_score_eg) / 256;

	/* Write debug if needed */
	if (dbg) {
		std::string output;

		output += "            | white | black |\n";
		output += "------------|-------|-------|\n";
		output += util::format("material_mg | %5d | %5d |\n", material_mg[piece::WHITE], material_mg[piece::BLACK]);
		output += util::format("material_eg | %5d | %5d |\n", material_eg[piece::WHITE], material_eg[piece::BLACK]);
		output += util::format("attackbonus | %5d | %5d |\n", attackbonus[piece::WHITE], attackbonus[piece::BLACK]);
		output += util::format("mobility_mg | %5d | %5d |\n", mobility_mg[piece::WHITE], mobility_mg[piece::BLACK]);
		output += util::format("mobility_eg | %5d | %5d |\n", mobility_eg[piece::WHITE], mobility_eg[piece::BLACK]);
		output += util::format("ctr_control | %5d | %5d |\n", center_control[piece::WHITE], center_control[piece::BLACK]);
		output += util::format("king_safety | %5d | %5d |\n", king_safety[piece::WHITE], king_safety[piece::BLACK]);
		output += util::format("blocked_pns | %5d | %5d |\n", blocking_pawns[piece::WHITE], blocking_pawns[piece::BLACK]);
		output += util::format("passed_pns  | %5d | %5d |\n", passed_pawns[piece::WHITE], passed_pawns[piece::BLACK]);
		output += util::format("adv_pawn_mg | %5d | %5d |\n", adv_pawn_mg[piece::WHITE], adv_pawn_mg[piece::BLACK]);
		output += util::format("adv_pawn_eg | %5d | %5d |\n", adv_pawn_eg[piece::WHITE], adv_pawn_eg[piece::BLACK]);
		output += util::format("adv_psd_mg  | %5d | %5d |\n", adv_passedpawn_mg[piece::WHITE], adv_passedpawn_mg[piece::BLACK]);
		output += util::format("adv_psd_eg  | %5d | %5d |\n", adv_passedpawn_eg[piece::WHITE], adv_passedpawn_eg[piece::BLACK]);
		output += util::format("adv_king_mg | %5d | %5d |\n", king_adv_mg[piece::WHITE], king_adv_mg[piece::BLACK]);
		output += util::format("adv_king_eg | %5d | %5d |\n", king_adv_eg[piece::WHITE], king_adv_eg[piece::BLACK]);
		output += util::format("phase       | %13d |\n", phase);

		*dbg = output;
	}

	return score + eval::TEMPO_BONUS;
}

zobrist::Key Position::get_tt_key() {
	return ply.back().key;
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
