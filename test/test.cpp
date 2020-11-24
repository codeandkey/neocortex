#include <gtest/gtest.h>

#include "../src/attacks.h"
#include "../src/bitboard.h"
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

/* Testing entry point */

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	attacks::init();
	tt::init(16); /* Keep small TT for testing. */
	zobrist::init();

	return RUN_ALL_TESTS();
}