/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "board.h"
#include "types.h"

#define NC_CASTLE_WHITE_K 1
#define NC_CASTLE_WHITE_Q 2
#define NC_CASTLE_BLACK_K 4
#define NC_CASTLE_BLACK_Q 8

#define NC_MAX_PL_MOVES 128
#define NC_MAX_PLY      512
#define NC_SEE_ILLEGAL     -100000

#define NC_EVAL_NOISE 100

typedef struct {
	ncMove last_move;
	ncSquare en_passant;
	int castle_rights;
	ncPiece captured_piece;
	ncSquare captured_square;
	int halfmove_clock;
	int fullmove_number;
	int check;
	int was_ep;
	int was_castle;
	ncHashKey key;
} ncPly;

typedef struct {
	ncBoard board;
	int ctm;
	int nply;
	ncPly ply[NC_MAX_PLY];
} ncPosition;

/**
 * Constructs a standard position.
 */
void ncPositionInit(ncPosition* p);

/**
 * Constructs a position from a FEN.
 *
 * @param fen Input FEN.
 */
int ncPositionFromFen(ncPosition* p, char* fen);

/**
 * Converts a position to a FEN.
 *
 * @return Number of characters copied.
 */
int ncPositionToFen(ncPosition* p, char* fen, int maxlen);

/**
 * Gets a reference to the position's current board.
 *
 * @return Board reference.
 */
static inline ncBoard* ncPositionGetBoard(ncPosition* p)
{
	return &p->board;
}

/**
 * Gets the current color to move.
 *
 * @return Color to move.
 */
static inline int ncPositionGetCTM(ncPosition* p)
{
	return p->ctm;
}

/**
 * Tries to make a move.
 * 
 * @param move Pseudolegal move.
 * @return 1 if move is legal, 0 otherwise.
 */
int ncPositionMakeMove(ncPosition* p, ncMove move);

/**
 * Unmakes the last move.
 */
void ncPositionUnmakeMove(ncPosition* p);

/**
 * Gets a mask of valid en passant squares.
 *
 * @return En-passant mask.
 */
static inline ncBitboard ncPositionEPMask(ncPosition* p)
{
	ncSquare sq = p->ply[p->nply - 1].en_passant;
	return ncSquareValid(sq) ? ncSquareMask(sq) : 0ULL;
}

/**
 * Gets the position's zobrist key for TT indexing.
 *
 * @return Zobrist key.
 */
static inline ncHashKey ncPositionGetKey(ncPosition* p)
{
	return p->ply[p->nply - 1].key;
}

/**
 * Test if the position is a check.
 *
 * @return 1 if color to move is in check, 0 otherwise.
 */
static inline int ncPositionIsCheck(ncPosition* p)
{
	return p->ply[p->nply - 1].check;
}

/**
 * Test if the last move made was a capture.
 *
 * @return 1 if a capture was made, 0 otherwise.
 */
static inline int ncPositionIsCapture(ncPosition* p)
{
	return ncPieceValid(p->ply[p->nply - 1].captured_piece);
}

/**
 * Test if the last move made was an en passant capture.
 *
 * @return 1 if last move was en passant, 0 otherwise.
 */
static inline int ncPositionIsEP(ncPosition* p)
{
	return p->ply[p->nply - 1].was_ep;
}

/**
 * Test if the last move made was a promotion.
 *
 * @return 1 if last move was a promotion, 0 otherwise.
 */
static inline int ncPositionIsPromotion(ncPosition* p)
{
	return ncPieceTypeValid(ncMovePtype(p->ply[p->nply - 1].last_move));
}

/**
 * Test if the last move made was a castle.
 *
 * @return 1 if last move was a castle, 0 otherwise.
 */
static inline int ncPositionIsCastle(ncPosition* p)
{
	return p->ply[p->nply - 1].was_castle;
}

/**
 * Gets the number of times the current position has occurred throughout the game.
 *
 * @return Number of instances. Will always be at least 1.
 */
int ncPositionRepCount(ncPosition* p);

/**
 * Gets the game's halfmove clock.
 *
 * @return Halfmove clock.
 */
static inline int ncPositionHalfmoveClock(ncPosition* p)
{
	return p->ply[p->nply - 1].halfmove_clock;
}

/**
 * Evaluates the current position.
 *
 * @param dbg String output pointer for debug information.
 * @return Evaluation score.
 */
int ncPositionEvaluate(ncPosition* p);

/**
 * Gets the pseudolegal moves for the position.
 * 
 * @param dst Buffer to fill with moves. Should be at least
 * NC_MAX_PL_MOVES in length.
 *
 * @return Number of moves generated.
 */
int ncPositionPLMoves(ncPosition* p, ncMove* dst);

/**
 * Gets the pseudolegal moves for the position (captures, checks, promotions, evasions (no castling)).
 * 
 * @param dst Buffer to fill with moves. Should be at least
 * NC_MAX_PL_MOVES in length.
 *
 * @return Number of moves generated.
 */
int ncPositionPLMovesQ(ncPosition* p, ncMove* dst);

/**
 * Gets evasion moves for a position.
 *
 * @param dst Buffer to fill with moves. Should be at least
 * NC_MAX_PL_MOVES in length.
 *
 * @return Number of moves generated.
 */
int ncPositionPLEvasions(ncPosition* p, ncMove* dst);

/**
 * Performs move ordering on a list of pseudolegal moves.
 *
 * @param moves Move list pointer.
 * @param num_moves Number of moves in the list.
 */
void ncPositionOrderMoves(ncPosition* p, ncMove* moves, int num);

/**
 * Performs static exchange evaluation on a capture.
 * 
 * @param capture Capturing move.
 */
int ncPositionSEECapture(ncPosition* p, ncMove capture);

/**
 * Performs static exchange evaluation on a square.
 *
 * @param sq Destination square.
 * @param col Attacking color.
 */
int ncPositionSEE(ncPosition* p, ncSquare sq, ncColor col);

/**
 * Get a printable debug dump of the position.
 * 
 * @return Number of characters copied.
 */
int ncPositionDump(ncPosition* p, char* out, int maxlen);

/**
 * Tests if the the position is a terminal. Returns the game result in <result>.
 *
 * @param pos Position
 * @param result Pointer to result value
 * @return 1 if terminal, 0 otherwis
 */
int ncPositionIsTerminal(ncPosition* pos, int* result);
