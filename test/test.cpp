#include "gtest/gtest.h"

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

/* Testing entry point */

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	attacks::init();
	tt::init(16); /* Keep small TT for testing. */
	zobrist::init();

	return RUN_ALL_TESTS();
}