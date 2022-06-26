/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "position.h"
#include "attacks.h"
#include "eval.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ncPositionInit(ncPosition* p)
{
	ncBoardStandard(&p->board);

	p->ctm = NC_WHITE;
	p->nply = 1;
	p->ply[0].captured_piece = NC_NULL;
	p->ply[0].captured_square = NC_NULL;
	p->ply[0].castle_rights = 0xF;
	p->ply[0].check = 0;
	p->ply[0].en_passant = NC_NULL;
	p->ply[0].key = ncBoardHashKey(&p->board);
	p->ply[0].halfmove_clock = 0;
	p->ply[0].fullmove_number = 1;
	p->ply[0].last_move = NC_NULL;
	p->ply[0].was_castle = 0;
	p->ply[0].was_ep = 0;
}

int ncPositionFromFen(ncPosition* p, char* in_fen)
{
	p->nply = 1;
	p->ply[0].captured_piece = NC_NULL;
	p->ply[0].captured_square = NC_NULL;
	p->ply[0].castle_rights = 0;
	p->ply[0].last_move = NC_NULL;
	p->ply[0].was_castle = 0;
	p->ply[0].was_ep = 0;
	p->ply[0].key = 0;

	char fen[100];
	char* save = NULL;

	strncpy(fen, in_fen, sizeof(fen));

	// Board
	if (ncBoardFromFen(&p->board, strtok_r(fen, " ", &save)))
		return -1;

	p->ply[0].key ^= ncBoardHashKey(&p->board);

	// Color to move
	switch (*strtok_r(NULL, " ", &save))
	{
		case 'w':
			p->ctm = NC_WHITE;
			break;
		case 'b':
			p->ctm = NC_BLACK;
			p->ply[0].key ^= ncZobristBlackToMove();
			break;
		default:
			return -1;
	}

	// Castling rights

	for (char* i = strtok_r(NULL, " ", &save); *i; ++i)
	{
		if (!i)
			return -1;

		switch (*i)
		{
			case 'K':
				p->ply[0].castle_rights |= NC_CASTLE_WHITE_K;
				break;
			case 'Q':
				p->ply[0].castle_rights |= NC_CASTLE_WHITE_Q;
				break;
			case 'k':
				p->ply[0].castle_rights |= NC_CASTLE_BLACK_K;
				break;
			case 'q':
				p->ply[0].castle_rights |= NC_CASTLE_BLACK_Q;
				break;
			case '-':
				continue;
		}
	}

	p->ply[0].key ^= ncZobristCastle(p->ply[0].castle_rights);

	// EP square
	char* epsq = strtok_r(NULL, " ", &save);

	if (!epsq)
		return -1;

	if (epsq[0] != '-')
		p->ply[0].en_passant = ncSquareAt(epsq[1] - '1', epsq[0] - 'a');

	// Halfmove clock
	char* hmc = strtok_r(NULL, " ", &save);

	if (!hmc)
		return -1;

	p->ply[0].halfmove_clock = strtol(hmc, NULL, 10);

	// Move number 
	char* mn = strtok_r(NULL, " ", &save);

	if (!mn)
		return -1;

	p->ply[0].fullmove_number = strtol(mn, NULL, 10);
	p->ply[0].check = ncBoardIsAttacked(&p->board, ncBoardPieceOcc(&p->board, NC_KING) & ncBoardColorOcc(&p->board, p->ctm), !p->ctm);

	return 0;
}

int ncPositionToFen(ncPosition* p, char* fen, int maxlen) {
	char tmpfen[100];
	int head = 0;
	ncPly* top = &p->ply[p->nply - 1];

	head += ncBoardToFen(&p->board, tmpfen, sizeof tmpfen);
	tmpfen[head++] = ' ';
	tmpfen[head++] = p->ctm == NC_WHITE ? 'w' : 'b';
	tmpfen[head++] = ' ';

	if (!top->castle_rights)
		tmpfen[head++] = '-';
	else
	{
		if (top->castle_rights & NC_CASTLE_WHITE_K) tmpfen[head++] = 'K';
		if (top->castle_rights & NC_CASTLE_WHITE_Q) tmpfen[head++] = 'Q';
		if (top->castle_rights & NC_CASTLE_BLACK_K) tmpfen[head++] = 'k';
		if (top->castle_rights & NC_CASTLE_BLACK_Q) tmpfen[head++] = 'q';
	}

	tmpfen[head++] = ' ';

	if (ncSquareValid(top->en_passant))
	{
		tmpfen[head++] = ncSquareFile(top->en_passant) + 'a';
		tmpfen[head++] = ncSquareRank(top->en_passant) + '1';
	}
	else
		tmpfen[head++] = '-';

	snprintf(tmpfen + head, sizeof(tmpfen) - head, " %d %d", top->halfmove_clock, top->fullmove_number);
	strncpy(fen, tmpfen, maxlen);

	return head < maxlen ? head : maxlen;
}

int ncPositionMakeMove(ncPosition* p, ncMove move)
{
	assert(p->nply < NC_MAX_PLY);

	ncPly* last = &p->ply[p->nply - 1];

	// Copy ply
	memcpy(&p->ply[p->nply], &p->ply[p->nply - 1], sizeof(ncPly));
	++p->nply;

	ncPly* top = &p->ply[p->nply - 1];

	top->last_move = move;

	if (p->ctm == NC_BLACK)
		++top->fullmove_number;

	++top->halfmove_clock;

	top->en_passant = NC_NULL;
	top->captured_piece = NC_NULL;
	top->captured_square = NC_NULL;
	top->was_ep = 0;
	top->was_castle = 0;

	ncSquare src_sq = ncMoveSrc(move), dst_sq = ncMoveDst(move);

	int src_piece = ncBoardRemove(&p->board, src_sq);
	int dst_piece = ncBoardGetPiece(&p->board, dst_sq);

	ncColor src_color = ncPieceColor(src_piece);
	ncPiece src_type  = ncPieceType(src_piece);

	assert(ncPieceColor(src_piece) == p->ctm);

	if (ncPieceType(src_piece) == NC_PAWN)
		top->halfmove_clock = 0;

	/* Check for EP capture */
	if (ncPieceType(src_piece) == NC_PAWN && dst_sq == last->en_passant) {
		/* EP, need to remove piece */
		int capture_square = ncSquareAt(
			ncSquareRank(src_sq),
			ncSquareFile(dst_sq)
		);

 		int removed = ncBoardRemove(&p->board, capture_square);

		top->captured_piece = removed;
		top->captured_square = capture_square;
		top->halfmove_clock = 0;
		top->was_ep = 1;

		assert(ncPieceType(removed) == NC_PAWN);
	}

	/* Move rook also if castling */
	if (ncPieceType(src_piece) == NC_KING && abs(ncSquareFile(ncMoveSrc(move)) - ncSquareFile(ncMoveDst(move))) > 1) {
		int is_ks = ncMoveDst(move) > ncMoveSrc(move);
		int castle_rank = (p->ctm == NC_WHITE) ? 0 : 7;
		int rook_file = is_ks ? 7 : 0;
		int rook_dstfile = is_ks ? 5 : 3;

		int rook_src = ncSquareAt(castle_rank, rook_file);
		int rook_dst = ncSquareAt(castle_rank, rook_dstfile);

		ncBoardPlace(&p->board, rook_dst, ncBoardRemove(&p->board, rook_src));

		top->was_castle = 1;
	}

	/* Test if destination is occupied */
	if (ncPieceValid(dst_piece)) {
		/* Normal capture */
		top->captured_piece = ncBoardReplace(&p->board, ncMoveDst(move), src_piece);
		top->captured_square = ncMoveDst(move);
		top->halfmove_clock = 0;

		/* no king captures */
		assert(ncPieceType(top->captured_piece) != NC_KING);
	} else {
		/* Quiet */
		ncBoardPlace(&p->board, ncMoveDst(move), src_piece);
	}

	/* Apply promotion */
	if (ncPieceValid(ncMovePtype(move))) {
		ncBoardReplace(&p->board, ncMoveDst(move), ncPieceMake(ncMovePtype(move), p->ctm));
	}

	/* Revoke castling rights */
	if (ncPieceType(src_piece) == NC_KING) {
		int rights = (p->ctm == NC_WHITE) ? 0x3 : 0xC;
		top->castle_rights &= ~rights;
	}

	ncBitboard white_ks_revoke = ncSquareMask(ncSquareAt(0, 4)) | ncSquareMask(ncSquareAt(0, 7));
	ncBitboard white_qs_revoke = ncSquareMask(ncSquareAt(0, 4)) | ncSquareMask(ncSquareAt(0, 0));
	ncBitboard black_ks_revoke = ncSquareMask(ncSquareAt(7, 4)) | ncSquareMask(ncSquareAt(7, 7));
	ncBitboard black_qs_revoke = ncSquareMask(ncSquareAt(7, 4)) | ncSquareMask(ncSquareAt(7, 0));

	ncBitboard src_dst_mask = ncSquareMask(ncMoveSrc(move)) | ncSquareMask(ncMoveDst(move));

	if (src_dst_mask & white_ks_revoke) {
		top->castle_rights &= ~NC_CASTLE_WHITE_K;
	}

	if (src_dst_mask & white_qs_revoke) {
		top->castle_rights &= ~NC_CASTLE_WHITE_Q;
	}

	if (src_dst_mask & black_ks_revoke) {
		top->castle_rights &= ~NC_CASTLE_BLACK_K;
	}

	if (src_dst_mask & black_qs_revoke) {
		top->castle_rights &= ~NC_CASTLE_BLACK_Q;
	}

	/* Update en passant */
	if (ncPieceType(src_piece) == NC_PAWN && abs(ncSquareRank(ncMoveDst(move)) - ncSquareRank(ncMoveSrc(move))) > 1) {
		int epsq = ncMoveDst(move) + ((p->ctm == NC_WHITE) ? NC_SOUTH : NC_NORTH);
		top->en_passant = epsq;

		if (p->ctm == NC_WHITE) {
			assert(epsq >= 16 && epsq <= 23);
		}
		else {
			assert(epsq >= 40 && epsq <= 47);
		}
	}

	/* Flip color to move */
	p->ctm = !p->ctm;

	/* Update zobrist key */
	top->key = 0;
	top->key ^= ncBoardHashKey(&p->board);

	if (ncSquareValid(top->en_passant))
		top->key ^= ncZobristEnPassant(top->en_passant);

	top->key ^= ncZobristCastle(top->castle_rights);

	if (p->ctm == NC_BLACK)
		top->key ^= ncZobristBlackToMove();

	top->check = -1;

	/* Check that king is not in attack */
	if (ncBoardIsAttacked(&p->board, ncBoardPieceOcc(&p->board, NC_KING) & ncBoardColorOcc(&p->board, !p->ctm), p->ctm))
		return 0;

	top->check = ncBoardIsAttacked(&p->board, ncBoardPieceOcc(&p->board, NC_KING) & ncBoardColorOcc(&p->board, p->ctm), !p->ctm);
	return 1;
}

void ncPositionUnmakeMove(ncPosition* p) {
	assert(p->nply);

	ncPly* last = &p->ply[--p->nply];
	ncMove move = last->last_move;

	/* Flip CTM early for readability */
	p->ctm = !p->ctm;

	/* Unpromote */
	if (ncPieceTypeValid(ncMovePtype(move))) {
		ncBoardReplace(&p->board, ncMoveDst(move), ncPieceMake(NC_PAWN, p->ctm));
	}

	/* Move piece back to source */
	int dst_piece = ncBoardRemove(&p->board, ncMoveDst(move));
	ncBoardPlace(&p->board, ncMoveSrc(move), dst_piece);

	/* Unmove rook in castling */
	if (ncPieceType(dst_piece) == NC_KING && abs(ncSquareFile(ncMoveSrc(move)) - ncSquareFile(ncMoveDst(move))) > 1) {
		int is_ks = ncMoveDst(move) > ncMoveSrc(move);
		int castle_rank = (p->ctm == NC_WHITE) ? 0 : 7;
		int rook_file = is_ks ? 5 : 3;
		int rook_dstfile = is_ks ? 7 : 0;

		int rook_src = ncSquareAt(castle_rank, rook_file);
		int rook_dst = ncSquareAt(castle_rank, rook_dstfile);

		ncBoardPlace(&p->board, rook_dst, ncBoardRemove(&p->board, rook_src));
	}

	/* Replace captured pieces */
	if (ncPieceValid(last->captured_piece)) {
		ncBoardPlace(&p->board, last->captured_square, last->captured_piece);
	}
}

int ncPositionPLMoves(ncPosition* p, ncMove* out)
{
	if (p->ply[p->nply - 1].check)
		return ncPositionPLEvasions(p, out);

	int count = 0;

	ncBitboard ctm = ncBoardColorOcc(&p->board, p->ctm);
	ncBitboard opp = ncBoardColorOcc(&p->board, !p->ctm);
	
	ncBitboard ep_mask = 0;
	ncSquare ep_square = p->ply[p->nply - 1].en_passant;

	if (ncSquareValid(ep_square)) {
		ep_mask = ncSquareMask(ep_square);
	}

	/* Pawn moves */
	ncBitboard pawns = ctm & ncBoardPieceOcc(&p->board, NC_PAWN);

	ncBitboard promoting_rank = (p->ctm == NC_WHITE) ? NC_RANK_7 : NC_RANK_2;
	ncBitboard starting_rank = (p->ctm == NC_WHITE) ? NC_RANK_2 : NC_RANK_7;

	int adv_dir = (p->ctm == NC_WHITE) ? NC_NORTH : NC_SOUTH;
	int left_dir = (p->ctm == NC_WHITE) ? NC_NORTHWEST : NC_SOUTHWEST;
	int right_dir = (p->ctm == NC_WHITE) ? NC_NORTHEAST : NC_SOUTHEAST;

	ncBitboard promoting_pawns = pawns & promoting_rank;

	/* Promoting left captures */
	ncBitboard promoting_left_cap = ncBitboardShift(promoting_pawns & ~NC_FILE_A, left_dir) & opp;

	while (promoting_left_cap) {
		int dst = ncBitboardPop(&promoting_left_cap);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_QUEEN);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_KNIGHT);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_ROOK);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_BISHOP);
	}

	/* Promoting right captures */
	ncBitboard promoting_right_cap = ncBitboardShift(promoting_pawns & ~NC_FILE_H, right_dir) & opp;

	while (promoting_right_cap) {
		int dst = ncBitboardPop(&promoting_right_cap);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_QUEEN);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_KNIGHT);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_ROOK);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_BISHOP);
	}

	/* Promoting advances */
	ncBitboard promoting_advances = ncBitboardShift(promoting_pawns, adv_dir) & ~ncBoardGlobalOcc(&p->board);

	while (promoting_advances) {
		int dst = ncBitboardPop(&promoting_advances);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_QUEEN);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_KNIGHT);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_ROOK);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_BISHOP);
	}

	/* Nonpromoting pawn moves */
	ncBitboard npm_pawns = pawns & ~promoting_pawns;

	/* Advances */
	ncBitboard npm_advances = ncBitboardShift(npm_pawns, adv_dir) & ~ncBoardGlobalOcc(&p->board);

	while (npm_advances) {
		int dst = ncBitboardPop(&npm_advances);
		out[count++] = ncMoveMake(dst - adv_dir, dst);
	}

	/* Left captures */
	ncBitboard npm_left_cap = ncBitboardShift(npm_pawns & ~NC_FILE_A, left_dir) & (opp | ep_mask);

	while (npm_left_cap) {
		int dst = ncBitboardPop(&npm_left_cap);
		out[count++] = ncMoveMake(dst - left_dir, dst);
	}

	/* Right captures */
	ncBitboard npm_right_cap = ncBitboardShift(npm_pawns & ~NC_FILE_H, right_dir) & (opp | ep_mask);

	while (npm_right_cap) {
		int dst = ncBitboardPop(&npm_right_cap);
		out[count++] = ncMoveMake(dst - right_dir, dst);
	}

	/* Jumps */
	ncBitboard npm_jumps = ncBitboardShift(pawns & starting_rank, adv_dir) & ~ncBoardGlobalOcc(&p->board);
	npm_jumps = ncBitboardShift(npm_jumps, adv_dir) & ~ncBoardGlobalOcc(&p->board);

	while (npm_jumps) {
		int dst = ncBitboardPop(&npm_jumps);
		out[count++] = ncMoveMake(dst - 2 * adv_dir, dst);
	}

	/* Queen moves */
	ncBitboard queens = ctm & ncBoardPieceOcc(&p->board, NC_QUEEN);
	
	while (queens) {
		int src = ncBitboardPop(&queens);
		ncBitboard moves = ncAttacksQueen(src, ncBoardGlobalOcc(&p->board)) & ~ctm;

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Rook moves */
	ncBitboard rooks = ctm & ncBoardPieceOcc(&p->board, NC_ROOK);

	while (rooks) {
		int src = ncBitboardPop(&rooks);
		ncBitboard moves = ncAttacksRook(src, ncBoardGlobalOcc(&p->board)) & ~ctm;

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Knight moves */
	ncBitboard knights = ctm & ncBoardPieceOcc(&p->board, NC_KNIGHT);

	while (knights) {
		int src = ncBitboardPop(&knights);
		ncBitboard moves = ncAttacksKnight(src) & ~ctm;

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Bishop moves */
	ncBitboard bishops = ctm & ncBoardPieceOcc(&p->board, NC_BISHOP);

	while (bishops) {
		int src = ncBitboardPop(&bishops);
		ncBitboard moves = ncAttacksBishop(src, ncBoardGlobalOcc(&p->board)) & ~ctm;

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* King moves */
	ncBitboard kings = ctm & ncBoardPieceOcc(&p->board, NC_KING);

	while (kings) {
		int src = ncBitboardPop(&kings);
		ncBitboard moves = ncAttacksKing(src) & ~ctm;

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Castling moves */

	/* We perform most of the legal move tests here, notably the noattack and occ tests */
	ncBitboard castle_rank = (p->ctm == NC_WHITE) ? NC_RANK_1 : NC_RANK_8;
	ncBitboard noattack_ks = castle_rank & (NC_FILE_E | NC_FILE_F | NC_FILE_G);
	ncBitboard noattack_qs = castle_rank & (NC_FILE_E | NC_FILE_D | NC_FILE_C);
	ncBitboard no_occ_ks = castle_rank & (NC_FILE_F | NC_FILE_G);
	ncBitboard no_occ_qs = castle_rank & (NC_FILE_B | NC_FILE_C | NC_FILE_D);

	int king_src = (p->ctm == NC_WHITE) ? ncSquareAt(0, 4) : ncSquareAt(7, 4);
	int ks_dst = (p->ctm == NC_WHITE) ? ncSquareAt(0, 6) : ncSquareAt(7, 6);
	int qs_dst = (p->ctm == NC_WHITE) ? ncSquareAt(0, 2) : ncSquareAt(7, 2);

	/* Kingside */
	if (p->ply[p->nply - 1].castle_rights & (1 << (p->ctm * 2))) {
		/* occ test */
		if (!(ncBoardGlobalOcc(&p->board) & no_occ_ks)) {
			/* noattack test */
			if (!ncBoardIsAttacked(&p->board, noattack_ks, !p->ctm)) {
				out[count++] = ncMoveMake(king_src, ks_dst);
			}
		}
	}

	/* Queenside */
	if (p->ply[p->nply - 1].castle_rights & (1 << (p->ctm * 2 + 1))) {
		/* occ test */
		if (!(ncBoardGlobalOcc(&p->board) & no_occ_qs)) {
			/* noattack test */
			if (!ncBoardIsAttacked(&p->board, noattack_qs, !p->ctm)) {
				out[count++] = ncMoveMake(king_src, qs_dst);
			}
		}
	}

	assert(count <= NC_MAX_PL_MOVES);

	return count;
}

int ncPositionPLEvasions(ncPosition* p, ncMove* out) {
	int count = 0;

	assert(p->ply[p->nply - 1].check == 1);

	ncBitboard ctm = ncBoardColorOcc(&p->board, p->ctm);
	ncBitboard opp = ncBoardColorOcc(&p->board, !p->ctm);

	/* King moves */
	ncBitboard kings = ctm & ncBoardPieceOcc(&p->board, NC_KING);

	/* Also grab attackers */
	int king_sq = ncBitboardUnmask(kings);
	ncBitboard attackers = ncBoardAttackers(&p->board, king_sq) & opp;

	while (kings) {
		int src = ncBitboardPop(&kings);
		ncBitboard moves = ncAttacksKing(src) & ~ctm;

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	if (ncBitboardPopcnt(attackers) > 1) {
		/* double check, king moves only */
		return count;
	}

	/* Only one attacker -- look for pieces that can capture it */
	int attacker_square = ncBitboardUnmask(attackers);

	/* If checker is sliding piece, add blocking moves */
	ncBitboard block_dsts = ncBitboardBetween(king_sq, attacker_square);

	ncBitboard ep_mask = 0;
	ncSquare ep_square = p->ply[p->nply - 1].en_passant;

	if (ncSquareValid(ep_square)) {
		ep_mask = ncSquareMask(ep_square);
	}

	/* Pawn moves */
	ncBitboard pawns = ctm & ncBoardPieceOcc(&p->board, NC_PAWN);

	ncBitboard promoting_rank = (p->ctm == NC_WHITE) ? NC_RANK_7 : NC_RANK_2;
	ncBitboard starting_rank = (p->ctm == NC_WHITE) ? NC_RANK_2 : NC_RANK_7;

	int adv_dir = (p->ctm == NC_WHITE) ? NC_NORTH : NC_SOUTH;
	int left_dir = (p->ctm == NC_WHITE) ? NC_NORTHWEST : NC_SOUTHWEST;
	int right_dir = (p->ctm == NC_WHITE) ? NC_NORTHEAST : NC_SOUTHEAST;
	ncBitboard promoting_pawns = pawns & promoting_rank;

	/* Promoting left captures */
	ncBitboard promoting_left_cap = ncBitboardShift(promoting_pawns & ~NC_FILE_A, left_dir) & attackers;

	while (promoting_left_cap) {
		int dst = ncBitboardPop(&promoting_left_cap);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_QUEEN);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_KNIGHT);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_ROOK);
		out[count++] = ncMoveMakeP(dst - left_dir, dst, NC_BISHOP);
	}

	/* Promoting right captures */
	ncBitboard promoting_right_cap = ncBitboardShift(promoting_pawns & ~NC_FILE_H, right_dir) & attackers;

	while (promoting_right_cap) {
		int dst = ncBitboardPop(&promoting_right_cap);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_QUEEN);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_KNIGHT);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_ROOK);
		out[count++] = ncMoveMakeP(dst - right_dir, dst, NC_BISHOP);
	}

	/* Promoting advances, blocking */
	ncBitboard promoting_advances = ncBitboardShift(promoting_pawns, adv_dir) & ~ncBoardGlobalOcc(&p->board) & block_dsts;

	while (promoting_advances) {
		int dst = ncBitboardPop(&promoting_advances);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_QUEEN);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_KNIGHT);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_ROOK);
		out[count++] = ncMoveMakeP(dst - adv_dir, dst, NC_BISHOP);
	}

	/* Nonpromoting pawn moves */
	ncBitboard npm_pawns = pawns & ~promoting_pawns;

	/* Blocking advances */
	ncBitboard npm_advances = ncBitboardShift(npm_pawns, adv_dir) & ~ncBoardGlobalOcc(&p->board) & block_dsts;

	while (npm_advances) {
		int dst = ncBitboardPop(&npm_advances);
		out[count++] = ncMoveMake(dst - adv_dir, dst);
	}

	/* Left captures */
	ncBitboard npm_left_cap = ncBitboardShift(npm_pawns & ~NC_FILE_A, left_dir) & (attackers | ep_mask);

	while (npm_left_cap) {
		int dst = ncBitboardPop(&npm_left_cap);
		out[count++] = ncMoveMake(dst - left_dir, dst);
	}

	/* Right captures */
	ncBitboard npm_right_cap = ncBitboardShift(npm_pawns & ~NC_FILE_H, right_dir) & (attackers | ep_mask);

	while (npm_right_cap) {
		int dst = ncBitboardPop(&npm_right_cap);
		out[count++] = ncMoveMake(dst - right_dir, dst);
	}

	/* Jumps */
	ncBitboard npm_jumps = ncBitboardShift(pawns & starting_rank, adv_dir) & ~ncBoardGlobalOcc(&p->board);
	npm_jumps = ncBitboardShift(npm_jumps, adv_dir) & ~ncBoardGlobalOcc(&p->board) & block_dsts;

	while (npm_jumps) {
		int dst = ncBitboardPop(&npm_jumps);
		out[count++] = ncMoveMake(dst - 2 * adv_dir, dst);
	}

	/* Queen moves */
	ncBitboard queens = ctm & ncBoardPieceOcc(&p->board, NC_QUEEN);

	while (queens) {
		int src = ncBitboardPop(&queens);
		ncBitboard moves = ncAttacksQueen(src, ncBoardGlobalOcc(&p->board)) & (block_dsts | attackers);

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Rook moves */
	ncBitboard rooks = ctm & ncBoardPieceOcc(&p->board, NC_ROOK);

	while (rooks) {
		int src = ncBitboardPop(&rooks);
		ncBitboard moves = ncAttacksRook(src, ncBoardGlobalOcc(&p->board)) & (block_dsts | attackers);

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Knight moves */
	ncBitboard knights = ctm & ncBoardPieceOcc(&p->board, NC_KNIGHT);

	while (knights) {
		int src = ncBitboardPop(&knights);
		ncBitboard moves = ncAttacksKnight(src) & (block_dsts | attackers);

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	/* Bishop moves */
	ncBitboard bishops = ctm & ncBoardPieceOcc(&p->board, NC_BISHOP);

	while (bishops) {
		int src = ncBitboardPop(&bishops);
		ncBitboard moves = ncAttacksBishop(src, ncBoardGlobalOcc(&p->board)) & (block_dsts | attackers);

		while (moves) {
			int dst = ncBitboardPop(&moves);
			out[count++] = ncMoveMake(src, dst);
		}
	}

	assert(count <= NC_MAX_PL_MOVES);
	return count;
}

void ncPositionOrderMoves(ncPosition* p, ncMove* moves, int num_moves) {
	int scores[NC_MAX_PL_MOVES] = { 0 };

	for (int i = 0; i < num_moves; ++i) {
		/* Assign move scores */

		// SEE on basic captures
		if (ncBoardColorOcc(&p->board, !p->ctm) & ncSquareMask(ncMoveDst(moves[i]))) scores[i] += ncPositionSEECapture(p, moves[i]);

		// SEE on EP captures
		if (ncPieceType(ncBoardGetPiece(&p->board, ncMoveSrc(moves[i]))) == NC_PAWN && ncMoveDst(moves[i]) == p->ply[p->nply - 1].en_passant) scores[i] += ncPositionSEECapture(p, moves[i]);
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

			ncMove tmove = moves[sind];
			moves[sind] = moves[i];
			moves[i] = tmove;
		}
	}
}

int ncPositionSEECapture(ncPosition* p, ncMove cap) {
	assert(!(ncSquareMask(ncMoveDst(cap)) & ncBoardPieceOcc(&p->board, NC_KING)));

	int ep = 0;
	int csq = ncMoveDst(cap);

	ncBitboard ep_mask = 0;

	if (ncSquareValid(p->ply[p->nply - 1].en_passant))
		ep_mask = ncSquareMask(p->ply[p->nply - 1].en_passant);

	if (ncPieceType(ncBoardGetPiece(&p->board, ncMoveSrc(cap))) == NC_PAWN && ncSquareMask(ncMoveDst(cap)) == ep_mask) {
		ep = 1;
		csq = ncMoveDst(cap) + ((p->ctm == NC_WHITE) ? NC_SOUTH : NC_NORTH);
	}

	int capped = NC_NULL;

	/* Make capture */
	if (ep) {
		capped = ncBoardRemove(&p->board, csq);
		ncBoardPlace(&p->board, ncMoveDst(cap), ncBoardRemove(&p->board, ncMoveSrc(cap)));
	} else {
		capped = ncBoardReplace(&p->board, ncMoveDst(cap), ncBoardRemove(&p->board, ncMoveSrc(cap)));
	}

	int ret = NC_MATERIAL_MG[ncPieceType(capped)] - ncPositionSEE(p, ncMoveDst(cap), !p->ctm);

	/* Undo capture */
	if (ep) {
		ncBoardPlace(&p->board, csq, capped);
		ncBoardPlace(&p->board, ncMoveSrc(cap), ncBoardRemove(&p->board, ncMoveDst(cap)));
	}
	else {
		ncBoardPlace(&p->board, ncMoveSrc(cap), ncBoardReplace(&p->board, ncMoveDst(cap), capped));
	}

	return ret;
}

int ncPositionSEE(ncPosition* p, int sq, int attacking_col) {
	ncBitboard ctm = ncBoardColorOcc(&p->board, attacking_col);
	int lva = NC_NULL;

#ifndef NDEBUG
	ncHashKey starting_bkey = ncBoardHashKey(&p->board);
#endif

	/* Find least valuable attacker */

	// Search for pawn attacks
	ncBitboard pawn_attackers = ncAttacksPawn(!attacking_col, sq) & ncBoardPieceOcc(&p->board, NC_PAWN) & ctm;

	if (pawn_attackers) {
		lva = ncBitboardPop(&pawn_attackers);
	}
	else {
		// Find bishop attackers
		ncBitboard bishop_attacks = ncAttacksBishop(sq, ncBoardGlobalOcc(&p->board));
		ncBitboard bishop_attackers = bishop_attacks & ctm & ncBoardPieceOcc(&p->board, NC_BISHOP);

		if (bishop_attackers) {
			lva = ncBitboardPop(&bishop_attackers);
		}
		else {
			// Find knight attackers
			ncBitboard knight_attackers = ncAttacksKnight(sq) & ctm & ncBoardPieceOcc(&p->board, NC_KNIGHT);

			if (knight_attackers) {
				lva = ncBitboardPop(&knight_attackers);
			}
			else {
				// Find rook attackers
				ncBitboard rook_attacks = ncAttacksRook(sq, ncBoardGlobalOcc(&p->board));
				ncBitboard rook_attackers = rook_attacks & ctm & ncBoardPieceOcc(&p->board, NC_ROOK);

				if (rook_attackers) {
					lva = ncBitboardPop(&rook_attackers);
				}
				else {
					// Find queen attackers
					ncBitboard queen_attackers = (bishop_attacks | rook_attacks) & ctm & ncBoardPieceOcc(&p->board, NC_QUEEN);

					if (queen_attackers) {
						lva = ncBitboardPop(&queen_attackers);
					}
					else {
						// Find king attackers
						ncBitboard king_attackers = ncAttacksKing(sq) & ctm & ncBoardPieceOcc(&p->board, NC_KING);

						if (king_attackers) {
							lva = ncBitboardPop(&king_attackers);
						}
					}
				}
			}
		}
	}

	if (ncSquareValid(lva)) {
		/* Remove LVA from board */
		int removed = ncBoardRemove(&p->board, lva);

		/* Perform capture with LVA, record material gain */
		int dst_removed = ncBoardReplace(&p->board, sq, removed);
		int dst_value = NC_MATERIAL_MG[ncPieceType(dst_removed)];

		int ret = dst_value - ncPositionSEE(p, sq, !attacking_col);

		/* Undo capture */
		ncBoardReplace(&p->board, sq, dst_removed);
		ncBoardPlace(&p->board, lva, removed);

		assert(ncBoardHashKey(&p->board) == starting_bkey);
		return ret;
	} else {
		// No valid attackers, SEE is 0
		assert(ncBoardHashKey(&p->board) == starting_bkey);
		return 0;
	}
}

int ncPositionEvaluate(ncPosition* p) {
	int opening = 0;
	int endgame = 0;

	int white_king_sq = ncBitboardUnmask(ncBoardColorOcc(&p->board, NC_WHITE) & ncBoardPieceOcc(&p->board, NC_KING));
	int black_king_sq = ncBitboardUnmask(ncBoardColorOcc(&p->board, NC_BLACK) & ncBoardPieceOcc(&p->board, NC_KING));

	ncBitboard wkattacks = ncAttacksKing(white_king_sq);
	ncBitboard bkattacks = ncAttacksKing(black_king_sq);

	// Start evaluation

	// Material values
	opening += ncBoardMaterialMG(&p->board);
	endgame += ncBoardMaterialEG(&p->board);

	// Center control
	static const ncSquare CENTER[4] = { 27, 28, 35, 36 };

	for (int i = 0; i < sizeof(CENTER) / sizeof(CENTER[0]); ++i)
	{
		int v = ncBoardGuard(&p->board, CENTER[i]);
		opening += v * NC_EVAL_CENTER_CONTROL_MG;
		endgame += v * NC_EVAL_CENTER_CONTROL_EG;
	}

	// King (un)safety
	ncBitboard white_ksq = wkattacks;
	ncBitboard black_ksq = bkattacks;

	while (white_ksq) {
		ncSquare ksq = ncBitboardPop(&white_ksq);
		int gv = ncBoardGuard(&p->board, ksq);

		// Dont give points for overprotecting own king squares
		if (gv > 0) {
			gv = 0;
		}

		opening += gv * NC_EVAL_KING_SAFETY_MG;
		endgame += gv * NC_EVAL_KING_SAFETY_EG;
	}

	while (black_ksq) {
		int ksq = ncBitboardPop(&black_ksq);
		int gv = ncBoardGuard(&p->board, ksq);

		// Dont give points for overprotecting own king squares
		if (gv < 0) {
			gv = 0;
		}

		opening += gv * NC_EVAL_KING_SAFETY_MG;
		endgame += gv * NC_EVAL_KING_SAFETY_EG;
	}

	// Development
	ncBitboard minor_pieces = ncBoardPieceOcc(&p->board, NC_KNIGHT) | ncBoardPieceOcc(&p->board, NC_BISHOP);

	static const ncBitboard white_dev_ranks = NC_RANK_3 | NC_RANK_4 | NC_RANK_5;
	static const ncBitboard black_dev_ranks = NC_RANK_4 | NC_RANK_5 | NC_RANK_6;

	int wdcount = ncBitboardPopcnt(minor_pieces & ncBoardColorOcc(&p->board, NC_WHITE) & white_dev_ranks);
	int bdcount = ncBitboardPopcnt(minor_pieces & ncBoardColorOcc(&p->board, NC_BLACK) & black_dev_ranks);

	opening += wdcount * NC_EVAL_DEVELOPMENT_MG;
	opening += bdcount * NC_EVAL_DEVELOPMENT_MG;
	endgame += wdcount * NC_EVAL_DEVELOPMENT_EG;
	endgame += bdcount * NC_EVAL_DEVELOPMENT_EG;

	// Edge knights
	ncBitboard edge_knights_mask = ncBoardPieceOcc(&p->board, NC_KNIGHT) & (NC_FILE_A | NC_FILE_H);

	int nedgew = ncBitboardPopcnt(edge_knights_mask & ncBoardColorOcc(&p->board, NC_WHITE));
	int nedgeb = ncBitboardPopcnt(edge_knights_mask & ncBoardColorOcc(&p->board, NC_BLACK));

	opening += nedgew * NC_EVAL_DEVELOPMENT_MG;
	opening += nedgeb * NC_EVAL_DEVELOPMENT_MG;
	endgame += nedgew * NC_EVAL_DEVELOPMENT_EG;
	endgame += nedgeb * NC_EVAL_DEVELOPMENT_EG;

	// Passed pawns
	ncBitboard wpassed, bpassed;

	wpassed = ncBoardPassers(&p->board, NC_WHITE);
	bpassed = ncBoardPassers(&p->board, NC_BLACK);

	// Passed pawn count bonus
	opening += ncBitboardPopcnt(wpassed) * NC_EVAL_PASSED_PAWNS_MG;
	opening += ncBitboardPopcnt(bpassed) * NC_EVAL_PASSED_PAWNS_MG;
	endgame += ncBitboardPopcnt(wpassed) * NC_EVAL_PASSED_PAWNS_EG;
	endgame += ncBitboardPopcnt(bpassed) * NC_EVAL_PASSED_PAWNS_EG;

	// First-rank kings
	ncBitboard wkmask = ncSquareMask(white_king_sq);
	ncBitboard bkmask = ncSquareMask(black_king_sq);

	if (ncSquareRank(white_king_sq) == 0)
	{
		opening += NC_EVAL_FIRST_RANK_KING_MG;
		endgame += NC_EVAL_FIRST_RANK_KING_EG;
		
		int ppc = ncBitboardPopcnt(wkattacks & ncBoardColorOcc(&p->board, NC_WHITE) & ncBoardPieceOcc(&p->board, NC_PAWN) & NC_RANK_2);

		opening += ppc * NC_EVAL_PAWNS_PROT_KING_MG;
		endgame += ppc * NC_EVAL_PAWNS_PROT_KING_EG;
	}

	if (ncSquareRank(black_king_sq) == 7)
	{
		opening -= NC_EVAL_FIRST_RANK_KING_MG;
		endgame -= NC_EVAL_FIRST_RANK_KING_EG;
		
		int ppc = ncBitboardPopcnt(bkattacks & ncBoardColorOcc(&p->board, NC_BLACK) & ncBoardPieceOcc(&p->board, NC_PAWN) & NC_RANK_2);

		opening -= ppc * NC_EVAL_PAWNS_PROT_KING_MG;
		endgame -= ppc * NC_EVAL_PAWNS_PROT_KING_EG;
	}

	// Advanced passed pawns
	ncBitboard tmp_passers;

	tmp_passers = wpassed;
	while (tmp_passers) {
		int p = ncBitboardPop(&tmp_passers);
		int dist = ncSquareRank(p) - 1;

		opening += dist * NC_EVAL_PASSED_PAWNS_MG;
		endgame += dist * NC_EVAL_PASSED_PAWNS_MG;
	}

	tmp_passers = bpassed;
	while (tmp_passers) {
		int p = ncBitboardPop(&tmp_passers);
		int dist = 6 - ncSquareRank(p);

		opening -= dist * NC_EVAL_PASSED_PAWNS_MG;
		endgame -= dist * NC_EVAL_PASSED_PAWNS_MG;
	}

	// File-based heuristics
	ncBitboard file = NC_FILE_A;

	for (int f = 0; f < 8; ++f) {
		ncBitboard file_pawns = ncBoardPieceOcc(&p->board, NC_PAWN) & file;
		file <<= 1;

		if (!file_pawns) {
			// Open file control by rooks/queens
			ncBitboard frooks = file & ncBoardPieceOcc(&p->board, NC_ROOK);
			ncBitboard fqueens = file & ncBoardPieceOcc(&p->board, NC_QUEEN);

			opening += ncBitboardPopcnt(frooks & ncBoardColorOcc(&p->board, NC_WHITE)) * NC_EVAL_OPEN_FILE_ROOK_MG;
			endgame += ncBitboardPopcnt(frooks & ncBoardColorOcc(&p->board, NC_WHITE)) * NC_EVAL_OPEN_FILE_ROOK_EG;
			opening += ncBitboardPopcnt(fqueens & ncBoardColorOcc(&p->board, NC_WHITE)) * NC_EVAL_OPEN_FILE_QUEEN_MG;
			endgame += ncBitboardPopcnt(fqueens & ncBoardColorOcc(&p->board, NC_WHITE)) * NC_EVAL_OPEN_FILE_QUEEN_EG;
			opening -= ncBitboardPopcnt(frooks & ncBoardColorOcc(&p->board, NC_BLACK)) * NC_EVAL_OPEN_FILE_ROOK_MG;
			endgame -= ncBitboardPopcnt(frooks & ncBoardColorOcc(&p->board, NC_BLACK)) * NC_EVAL_OPEN_FILE_ROOK_EG;
			opening -= ncBitboardPopcnt(fqueens & ncBoardColorOcc(&p->board, NC_BLACK)) * NC_EVAL_OPEN_FILE_QUEEN_MG;
			endgame -= ncBitboardPopcnt(fqueens & ncBoardColorOcc(&p->board, NC_BLACK)) * NC_EVAL_OPEN_FILE_QUEEN_EG;
		}

		// Doubled pawns
		int n_white_pawns = ncBitboardPopcnt(file_pawns & ncBoardColorOcc(&p->board, NC_WHITE));
		int n_black_pawns = ncBitboardPopcnt(file_pawns & ncBoardColorOcc(&p->board, NC_BLACK));

		opening += (n_white_pawns - 1) * NC_EVAL_DOUBLED_PAWNS_MG;
		endgame += (n_white_pawns - 1) * NC_EVAL_DOUBLED_PAWNS_EG;
		opening -= (n_black_pawns - 1) * NC_EVAL_DOUBLED_PAWNS_MG;
		endgame -= (n_black_pawns - 1) * NC_EVAL_DOUBLED_PAWNS_EG;
	}

	// Chained pawns
	ncBitboard w_pawns = ncBoardPieceOcc(&p->board, NC_PAWN) & ncBoardColorOcc(&p->board, NC_WHITE);
	ncBitboard b_pawns = ncBoardPieceOcc(&p->board, NC_PAWN) & ncBoardColorOcc(&p->board, NC_BLACK);

	ncBitboard wpawnchain = (ncBitboardShift(w_pawns & ~NC_FILE_A, NC_NORTHWEST) | ncBitboardShift(w_pawns & ~NC_FILE_H, NC_NORTHEAST)) & w_pawns;
	ncBitboard bpawnchain = (ncBitboardShift(b_pawns & ~NC_FILE_A, NC_NORTHWEST) | ncBitboardShift(w_pawns & ~NC_FILE_H, NC_NORTHEAST)) & w_pawns;

	int nwpawnchain = ncBitboardPopcnt(wpawnchain);
	int nbpawnchain = ncBitboardPopcnt(bpawnchain);

	opening += nwpawnchain * NC_EVAL_PAWN_CHAIN_MG;
	endgame += nwpawnchain * NC_EVAL_PAWN_CHAIN_EG;
	opening -= nbpawnchain * NC_EVAL_PAWN_CHAIN_MG;
	endgame -= nbpawnchain * NC_EVAL_PAWN_CHAIN_EG;

	// Isolated pawns
	int nwisolated = ncBitboardPopcnt(ncBoardIsolated(&p->board, NC_WHITE));
	int nbisolated = ncBitboardPopcnt(ncBoardIsolated(&p->board, NC_BLACK));

	opening += nwisolated * NC_EVAL_ISOLATED_PAWNS_MG;
	endgame += nwisolated * NC_EVAL_ISOLATED_PAWNS_EG;
	opening -= nbisolated * NC_EVAL_ISOLATED_PAWNS_MG;
	endgame -= nbisolated * NC_EVAL_ISOLATED_PAWNS_EG;

	// Backward pawns
	int nwbackward = ncBitboardPopcnt(ncBoardBackward(&p->board, NC_WHITE));
	int nbbackward = ncBitboardPopcnt(ncBoardBackward(&p->board, NC_BLACK));

	opening += nwbackward * NC_EVAL_BACKWARD_PAWNS_MG;
	endgame += nwbackward * NC_EVAL_BACKWARD_PAWNS_EG;
	opening -= nbbackward * NC_EVAL_BACKWARD_PAWNS_MG;
	endgame -= nbbackward * NC_EVAL_BACKWARD_PAWNS_EG;

	/* Compute game phase */
	int phase = NC_EVAL_PHASE_TOTAL;

	for (int t = 0; t < 5; ++t) {
		phase -= ncBitboardPopcnt(ncBoardPieceOcc(&p->board, t)) * NC_EVAL_PHASE_VALS[t];
	}

	phase = (phase * 256) / NC_EVAL_PHASE_TOTAL;

	// Taper eval
	return ((opening * (256 - phase)) + (endgame * phase)) / 256;
}

int ncPositionNumRepetitions(ncPosition* p) {
	int res = 0;

	for (size_t i = 0; i < p->nply; ++i)
		res += (p->ply[i].key == p->ply[p->nply - 1].key);

	return res;
}

int ncPositionIsTerminal(ncPosition* pos, int* result)
{
	// Check for terminal draws
    if (pos->ply[pos->nply - 1].halfmove_clock >= 50)
    {
		*result = 0;
        return 1;
    }

    if (ncPositionRepCount(pos) >= 3)
    {
		*result = 0;
        return 1;
    }

	ncMove moves[NC_MAX_PL_MOVES];
	int n_pl_moves;

	n_pl_moves = ncPositionPLMoves(pos, moves);

	for (int i = 0; i < n_pl_moves; ++i)
	{
		if (ncPositionMakeMove(pos, moves[i]))
		{
			ncPositionUnmakeMove(pos);
			return 0;
		}

		ncPositionUnmakeMove(pos);
	}
	
	if (!ncPositionIsCheck(pos))
		*result = 0;

	*result = (pos->ctm == NC_WHITE) ? -1 : 1;
	return 1;
}

int ncPositionRepCount(ncPosition* p)
{
	int count = 0;
	ncHashKey key = p->ply[p->nply - 1].key;

	for (int i = p->nply - 2; i >= 0; --i)
		if (p->ply[i].key == key)
			++count;

	return count;
}
