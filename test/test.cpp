#include <gtest/gtest.h>

#include "../src/attacks.h"
#include "../src/bitboard.h"
#include "../src/board.h"
#include "../src/eval_consts.h"
#include "../src/piece.h"
#include "../src/tt.h"
#include "../src/zobrist.h"

using namespace neocortex;

/**
		 * AttacksTest: tests for attack lookups in attacks.cpp
		 */
TEST(AttacksTest, PawnAttacks) {
	EXPECT_EQ(attacks::pawn(piece::WHITE, 0), 1ULL << 9);
	EXPECT_EQ(attacks::pawn(piece::WHITE, 5), (1ULL << 12) | (1ULL << 14));
	EXPECT_EQ(attacks::pawn(piece::BLACK, 21), (1ULL << 12) | (1ULL << 14));
	EXPECT_EQ(attacks::pawn(piece::BLACK, 63), 1ULL << 54);
}

TEST(AttacksTest, PawnFrontSpans) {
	EXPECT_EQ(attacks::pawn_frontspans(piece::WHITE, 0), FILE_A & ~RANK_1);
	EXPECT_EQ(attacks::pawn_frontspans(piece::BLACK, 63), FILE_H & ~RANK_8);
}

TEST(AttacksTest, PawnAttackSpans) {
	EXPECT_EQ(attacks::pawn_attackspans(piece::WHITE, 12), (FILE_D | FILE_F) & ~(RANK_1 | RANK_2));
	EXPECT_EQ(attacks::pawn_attackspans(piece::BLACK, 52), (FILE_D | FILE_F) & ~(RANK_7 | RANK_8));
}

TEST(AttacksTest, KingAttacks) {
	EXPECT_EQ(attacks::king(0), 0x302);
	EXPECT_EQ(attacks::king(12), (FILE_D | FILE_E | FILE_F) & (RANK_1 | RANK_2 | RANK_3) & ~(1ULL << 12));
}

TEST(AttacksTest, KnightAttacks) {
	EXPECT_EQ(attacks::knight(0), (1ULL << 10) | (1ULL << 17));
	EXPECT_EQ(attacks::knight(14), (1ULL << 4) | (1ULL << 20) | (1ULL << 29) | (1ULL << 31));
}

TEST(AttacksTest, BishopAttacks) {
	EXPECT_EQ(attacks::bishop(0, 0), 0x8040201008040200);
}

TEST(AttacksTest, RookAttacks) {
	EXPECT_EQ(attacks::rook(0, 0), (FILE_A | RANK_1) & ~1);
	EXPECT_EQ(attacks::rook(7, 0), (FILE_H | RANK_1) & ~(1ULL << 7));
	EXPECT_EQ(attacks::rook(12, 0), (FILE_E | RANK_2) & ~(1ULL << 12));
}

TEST(AttacksTest, QueenAttacks) {
	EXPECT_EQ(attacks::queen(0, 0), (0x8040201008040200 | RANK_1 | FILE_A) & ~1);
}

/**
 * BitboardTest: tests for bitboard operations in bitboard.cpp
 */

TEST(BitboardTest, ToString) {
	EXPECT_EQ(
		bb::to_string(0xFF),
		std::string("........\n........\n........\n........\n........\n........\n........\n11111111\n")
	);
}

TEST(BitboardTest, GetLsb) {
	EXPECT_EQ(bb::getlsb(0x100), 8);
	EXPECT_EQ(bb::getlsb(0x200), 9);
	EXPECT_EQ(bb::getlsb(0x400), 10);
}

TEST(BitboardTest, PopLsb) {
	bitboard b = 0x400;

	EXPECT_EQ(bb::poplsb(b), 10);
	EXPECT_EQ(b, 0ULL);
}

TEST(BitboardTest, Shift) {
	EXPECT_EQ(bb::shift(1, NORTH), (1ULL << 8));
	EXPECT_EQ(bb::shift(1, EAST), (1ULL << 1));
	EXPECT_EQ(bb::shift(1ULL << 8, SOUTH), 1ULL);
}

TEST(BitboardTest, Mask) {
	EXPECT_EQ(bb::mask(6), 1ULL << 6);
	EXPECT_EQ(bb::mask(12), 1ULL << 12);
	EXPECT_EQ(bb::mask(24), 1ULL << 24);
}

TEST(BitboardTest, Popcount) {
	EXPECT_EQ(bb::popcount(0xF), 4);
	EXPECT_EQ(bb::popcount(0xFFF), 12);
}

TEST(BitboardTest, Rank) {
	EXPECT_EQ(bb::rank(0), RANK_1);
	EXPECT_EQ(bb::rank(4), RANK_5);
	EXPECT_EQ(bb::rank(7), RANK_8);
}

TEST(BitboardTest, File) {
	EXPECT_EQ(bb::file(0), FILE_A);
	EXPECT_EQ(bb::file(4), FILE_E);
	EXPECT_EQ(bb::file(7), FILE_H);
}

/**
 * BoardTest: tests for board data type in board.cpp
 */

TEST(BoardTest, Place) {
	Board b = Board::standard();

	b.place(16, piece::make_piece(piece::WHITE, piece::PAWN));

	EXPECT_EQ(b.to_uci(), "rnbqkbnr/pppppppp/8/8/8/P7/PPPPPPPP/RNBQKBNR");
}

TEST(BoardTest, Remove) {
	Board b = Board::standard();

	EXPECT_EQ(b.remove(0), piece::make_piece(piece::WHITE, piece::ROOK));
	EXPECT_EQ(b.to_uci(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1NBQKBNR");
}

TEST(BoardTest, Replace) {
	Board b = Board::standard();

	EXPECT_EQ(b.replace(0, piece::make_piece(piece::BLACK, piece::ROOK)), piece::make_piece(piece::WHITE, piece::ROOK));
	EXPECT_EQ(b.to_uci(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/rNBQKBNR");
}

TEST(BoardTest, ToUci) {
	EXPECT_EQ(Board::standard().to_uci(), std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));
}

TEST(BoardTest, ParseUci) {
	std::vector<std::string> uci_list;

	uci_list.push_back("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8");
	uci_list.push_back("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1");
	uci_list.push_back("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R");
	uci_list.push_back("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R");

	for (auto i : uci_list) {
		EXPECT_EQ(Board(i).to_uci(), i);
	}
}

TEST(BoardTest, ToPretty) {
	EXPECT_EQ(Board::standard().to_pretty(), "rnbqkbnr\npppppppp\n........\n........\n........\n........\nPPPPPPPP\nRNBQKBNR\n");
}

TEST(BoardTest, GlobalOcc) {
	EXPECT_EQ(Board::standard().get_global_occ(), 0xFFFF00000000FFFF);
}

TEST(BoardTest, ColorOcc) {
	EXPECT_EQ(Board::standard().get_color_occ(piece::WHITE), 0x000000000000FFFF);
	EXPECT_EQ(Board::standard().get_color_occ(piece::BLACK), 0xFFFF000000000000);
}

TEST(BoardTest, PieceOcc) {
	EXPECT_EQ(Board::standard().get_piece_occ(piece::PAWN), RANK_2 | RANK_7);
	EXPECT_EQ(Board::standard().get_piece_occ(piece::ROOK), (RANK_1 | RANK_8) & (FILE_A | FILE_H));
	EXPECT_EQ(Board::standard().get_piece_occ(piece::KNIGHT), (RANK_1 | RANK_8) & (FILE_B | FILE_G));
	EXPECT_EQ(Board::standard().get_piece_occ(piece::BISHOP), (RANK_1 | RANK_8) & (FILE_C | FILE_F));
	EXPECT_EQ(Board::standard().get_piece_occ(piece::QUEEN), (RANK_1 | RANK_8) & FILE_D);
	EXPECT_EQ(Board::standard().get_piece_occ(piece::KING), (RANK_1 | RANK_8) & FILE_E);
}

TEST(BoardTest, GetPiece) {
	EXPECT_EQ(Board::standard().get_piece(0), piece::make_piece(piece::WHITE, piece::ROOK));
	EXPECT_EQ(Board::standard().get_piece(4), piece::make_piece(piece::WHITE, piece::KING));
}

TEST(BoardTest, GetTTKey) {
	Board b = Board::standard();
	zobrist::Key k = b.get_tt_key();
	int p = piece::make_piece(piece::BLACK, piece::KNIGHT);

	b.place(32, p);

	EXPECT_EQ(b.get_tt_key(), k ^ zobrist::piece(32, p));
}

TEST(BoardTest, AttacksOn) {
	EXPECT_EQ(Board::standard().attacks_on(16), (1ULL << 9) | (1ULL << 1));
	EXPECT_EQ(Board::standard().attacks_on(18), (1ULL << 9) | (1ULL << 1) | (1ULL << 11));
}

TEST(BoardTest, GuardValue) {
	EXPECT_EQ(Board::standard().guard_value(0), 0);
	EXPECT_EQ(Board::standard().guard_value(32), 0);

	EXPECT_EQ(
		Board::standard().guard_value(16),
		eval::GUARD_VALUES[piece::make_piece(piece::WHITE, piece::PAWN)] + eval::GUARD_VALUES[piece::make_piece(piece::WHITE, piece::KNIGHT)]
	);
}

TEST(BoardTest, MaskIsAttacked) {
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_4, piece::WHITE), false);
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_4, piece::BLACK), false);

	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_1, piece::WHITE), true);
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_1, piece::BLACK), false);

	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_8, piece::BLACK), true);
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_8, piece::WHITE), false);
}

/* Testing entry point */

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	attacks::init();
	tt::init(16); /* Keep small TT (16mb) for testing. */
	zobrist::init();

	return RUN_ALL_TESTS();
}