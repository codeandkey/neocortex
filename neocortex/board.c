/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "board.h"
#include "eval.h"
#include "zobrist.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

void ncBoardInit(ncBoard* b)
{
	memset(b, 0, sizeof(*b));

	for (int i = 0; i < 64; ++i) {
		b->state[i] = -1;
	}
}

int ncBoardFromFen(ncBoard* b, const char* fen)
{
	char localfen[80];
	strncpy(localfen, fen, sizeof(localfen));

	ncBoardInit(b);

	int r = 7;
	for (char* rank = strtok(localfen, "/"); rank; rank = strtok(NULL, "/"))
	{
		if (r < 0)
			return -1;

		int f;
		for (f = 0; f < 8; ++f)
		{
			if (isdigit(rank[f])) 
			{
				f += rank[f] - '1';
			} else
			{
				ncPiece decoded = ncPieceFromChar(rank[f]);

				if (decoded == NC_NULL)
					return -1;

				ncBoardPlace(b, ncSquareAt(r, f), decoded);
			}
		}

		if (f != 8)
			return -1;

		--r;
	}

	return 0;
}

void ncBoardStandard(ncBoard* b)
{
	ncBoardFromFen(b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

void ncBoardPlace(ncBoard* b, ncSquare sq, ncPiece p)
{
	assert(ncSquareValid(sq));
	assert(ncPieceValid(p));
	assert(!ncPieceValid(b->state[sq]));

	b->state[sq] = p;

	ncBitboard mask = ncSquareMask(sq);

	b->global_occ ^= mask;
	b->piece_occ[ncPieceType(p)] ^= mask;
	b->color_occ[ncPieceColor(p)] ^= mask;
	b->key ^= ncZobristPiece(sq, p);
	b->mat_mg += ncMaterialValueMG(p);
	b->mat_eg += ncMaterialValueEG(p);
}

ncPiece ncBoardRemove(ncBoard* b, ncSquare sq) {
	assert(ncSquareValid(sq));

	ncPiece res = b->state[sq];

	assert(ncPieceValid(res));

	ncBitboard mask = ncSquareMask(sq);

	b->global_occ ^= mask;
	b->piece_occ[ncPieceType(res)] ^= mask;
	b->color_occ[ncPieceColor(res)] ^= mask;
	b->key ^= ncZobristPiece(sq, res);

	b->state[sq] = NC_NULL;

	b->mat_mg -= ncMaterialValueMG(res);
	b->mat_eg -= ncMaterialValueEG(res);

	return res;
}

ncPiece ncBoardReplace(ncBoard* b, ncSquare sq, ncPiece p) {
	assert(ncSquareValid(sq));

	ncPiece res = b->state[sq];

	assert(ncPieceValid(res));

	ncBitboard mask = ncSquareMask(sq);

	b->piece_occ[ncPieceType(res)] ^= mask;
	b->color_occ[ncPieceColor(res)] ^= mask;
	b->piece_occ[ncPieceType(p)] ^= mask;
	b->color_occ[ncPieceColor(p)] ^= mask;

	b->key ^= ncZobristPiece(sq, res);
	b->key ^= ncZobristPiece(sq, p);

	b->mat_mg -= ncMaterialValueMG(res);
	b->mat_eg -= ncMaterialValueEG(res);
	b->mat_mg += ncMaterialValueMG(p);
	b->mat_eg += ncMaterialValueEG(p);

	b->state[sq] = p;

	return res;
}

int ncBoardToFen(ncBoard* b, char* out, int maxlen)
{
	int null_count = 0;
	int head = 0;
	char localbuf[80];

	memset(localbuf, 0, sizeof localbuf);

	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f < 8; ++f) {
			ncSquare sq = ncSquareAt(r, f);

			if (b->state[sq] == NC_NULL) {
				null_count++;
			} else {
				if (null_count > 0) {
					localbuf[head++] = '0' + null_count;
					null_count = 0;
				}

				localbuf[head++] = ncPieceToChar(b->state[sq]);
			}
		}

		if (null_count > 0) {
			localbuf[head++] = '0' + null_count;
			null_count = 0;
		}

		if (r) localbuf[head++] += '/';
	}

	strncpy(out, localbuf, head);
	return head < maxlen ? head : maxlen;
}

int ncBoardWritePretty(ncBoard* b, char* out, int maxlen) {
	int head = 0;
	char localbuf[70];

	for (int r = 7; r >= 0 && head < maxlen - 1; --r) {
		for (int f = 0; f < 8 && head < maxlen - 1; ++f) {
			ncSquare sq = ncSquareAt(r, f);

			if (!ncPieceValid(b->state[sq])) {
				localbuf[head++] = '.';
			} else {
				localbuf[head++] = ncPieceToChar(b->state[sq]);
			}
		}

		localbuf[head++] = '\n';
	}

	strncpy(out, localbuf, maxlen);

	return head < maxlen ? head : maxlen;
}

ncBitboard ncBoardAttackers(ncBoard* b, ncSquare sq) {
	assert(ncSquareValid(sq));

	ncBitboard white_pawns = b->piece_occ[NC_PAWN] & b->color_occ[NC_WHITE];
	ncBitboard black_pawns = b->piece_occ[NC_PAWN] & b->color_occ[NC_BLACK];
	ncBitboard rooks_queens = b->piece_occ[NC_ROOK] | b->piece_occ[NC_QUEEN];
	ncBitboard bishops_queens = b->piece_occ[NC_BISHOP] | b->piece_occ[NC_QUEEN];

	return (ncAttacksPawn(NC_WHITE, sq) & black_pawns) |
		(ncAttacksPawn(NC_BLACK, sq) & white_pawns) |
		(ncAttacksKnight(sq) & b->piece_occ[NC_KNIGHT]) |
		(ncAttacksBishop(sq, b->global_occ) & bishops_queens) |
		(ncAttacksRook(sq, b->global_occ) & rooks_queens) |
		(ncAttacksKing(sq) & b->piece_occ[NC_KING]);
}

int ncBoardGuard(ncBoard* b, ncSquare sq) {
	int val = 0;
	ncBitboard att = ncBoardAttackers(b, sq);

	while (att) {
		val += ncGuardValue(b->state[ncBitboardPop(&att)]);
	}

	return val;
}

int ncBoardIsAttacked(ncBoard* b, ncBitboard mask, ncColor col) {
	ncBitboard opp = b->color_occ[col];

	while (mask) {
		if (ncBoardAttackers(b, ncBitboardPop(&mask)) & opp) return 1;
	}

	return 0;
}

ncBitboard ncBoardAllspans(ncBoard* b, ncColor col) {
	ncBitboard out = 0;
	ncBitboard pawns = b->piece_occ[NC_PAWN] & b->color_occ[col];

	while (pawns) {
		int p = ncBitboardPop(&pawns);

		out |= ncPawnAttackspans(col, p);
		out |= ncPawnFrontspans(col, p);
	}

	return out;
}

ncBitboard ncBoardFrontspans(ncBoard* b, ncColor col) {
	ncBitboard out = 0;
	ncBitboard pawns = b->piece_occ[NC_PAWN] & b->color_occ[col];

	while (pawns) {
		int p = ncBitboardPop(&pawns);

		out |= ncPawnFrontspans(col, p);
	}

	return out;
}

ncBitboard ncBoardAttackspans(ncBoard* b, ncColor col) {
	ncBitboard out = 0;
	ncBitboard pawns = b->piece_occ[NC_PAWN] & b->color_occ[col];

	while (pawns) {
		int p = ncBitboardPop(&pawns);

		out |= ncPawnAttackspans(col, p);
	}

	return out;
}

ncBitboard ncBoardIsolated(ncBoard* b, ncColor col) {
	ncBitboard pawns = b->piece_occ[NC_PAWN] & b->color_occ[col];
	ncBitboard out = 0ULL;

	while (pawns) {
		int p = ncBitboardPop(&pawns);

		if (!(ncSquareNeighborFiles(p) & pawns)) {
			out |= ncSquareMask(p);
		}
	}
	
	return out;
}

ncBitboard ncBoardBackward(ncBoard* b, ncColor col) {
	ncBitboard pawns = b->piece_occ[NC_PAWN] & b->color_occ[col];
	ncBitboard stops = ncBitboardShift(pawns, (col == NC_WHITE) ? NC_NORTH : NC_SOUTH);

	ncBitboard opp_pawns = b->piece_occ[NC_PAWN] & b->color_occ[!col];
	ncBitboard opp_attacks = ncBitboardShift(opp_pawns & ~NC_FILE_A, (col == NC_WHITE) ? NC_NORTHWEST : NC_SOUTHWEST);

	opp_attacks |= ncBitboardShift(opp_pawns & ~NC_FILE_H, (col == NC_WHITE) ? NC_NORTHEAST : NC_SOUTHEAST);

	stops &= ~ncBoardAttackspans(b, col);
	stops &= opp_attacks;

	return ncBitboardShift(stops, (col == NC_WHITE) ? NC_SOUTH : NC_NORTH);
}

ncBitboard ncBoardPassers(ncBoard* b, ncColor col)
{
	return ~ncBoardAllspans(b, !col) & b->piece_occ[NC_PAWN] & b->color_occ[col];
}
