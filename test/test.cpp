/* vim: set ts=4 sw=4 noet: */

#include <gtest/gtest.h>

#include "../src/attacks.h"
#include "../src/bitboard.h"
#include "../src/board.h"
#include "../src/color.h"
#include "../src/type.h"
#include "../src/log.h"
#include "../src/perft.h"
#include "../src/piece.h"
#include "../src/zobrist.h"

using namespace neocortex;

/**
 * AttacksTest: tests for attack lookups in attacks.cpp
 */

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

TEST(BitboardTest, Between) {
	EXPECT_EQ(bb::between(42, 60), 1ULL << 51);
	EXPECT_EQ(bb::between(0, 56), FILE_A & ~(RANK_1 | RANK_8));
	EXPECT_EQ(bb::between(0, 7), RANK_1 & ~(FILE_A | FILE_H));

	/* Test symmetry in between lookup */
	for (int src = 0; src < 64; ++src) {
		for (int dst = 0; dst < 64; ++dst) {
			EXPECT_EQ(bb::between(src, dst), bb::between(dst, src));
		}
	}
}

/**
 * BoardTest: tests for board data type in board.cpp
 */

TEST(BoardTest, Place) {
	Board b = Board::standard();

	b.place(16, piece::make(color::WHITE, type::PAWN));

	EXPECT_EQ(b.to_uci(), "rnbqkbnr/pppppppp/8/8/8/P7/PPPPPPPP/RNBQKBNR");
}

TEST(BoardTest, Remove) {
	Board b = Board::standard();

	EXPECT_EQ(b.remove(0), piece::make(color::WHITE, type::ROOK));
	EXPECT_EQ(b.to_uci(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1NBQKBNR");
}

TEST(BoardTest, Replace) {
	Board b = Board::standard();

	EXPECT_EQ(b.replace(0, piece::make(color::BLACK, type::ROOK)), piece::make(color::WHITE, type::ROOK));
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
	EXPECT_EQ(Board::standard().get_color_occ(color::WHITE), 0x000000000000FFFF);
	EXPECT_EQ(Board::standard().get_color_occ(color::BLACK), 0xFFFF000000000000);
}

TEST(BoardTest, PieceOcc) {
	EXPECT_EQ(Board::standard().get_piece_occ(type::PAWN), RANK_2 | RANK_7);
	EXPECT_EQ(Board::standard().get_piece_occ(type::ROOK), (RANK_1 | RANK_8) & (FILE_A | FILE_H));
	EXPECT_EQ(Board::standard().get_piece_occ(type::KNIGHT), (RANK_1 | RANK_8) & (FILE_B | FILE_G));
	EXPECT_EQ(Board::standard().get_piece_occ(type::BISHOP), (RANK_1 | RANK_8) & (FILE_C | FILE_F));
	EXPECT_EQ(Board::standard().get_piece_occ(type::QUEEN), (RANK_1 | RANK_8) & FILE_D);
	EXPECT_EQ(Board::standard().get_piece_occ(type::KING), (RANK_1 | RANK_8) & FILE_E);
}

TEST(BoardTest, GetPiece) {
	EXPECT_EQ(Board::standard().get_piece(0), piece::make(color::WHITE, type::ROOK));
	EXPECT_EQ(Board::standard().get_piece(4), piece::make(color::WHITE, type::KING));
}

TEST(BoardTest, GetTTKey) {
	Board b = Board::standard();
	zobrist::Key k = b.get_tt_key();
	int p = piece::make(color::BLACK, type::KNIGHT);

	b.place(32, p);

	EXPECT_EQ(b.get_tt_key(), k ^ zobrist::piece(32, p));
}

TEST(BoardTest, AttacksOn) {
	EXPECT_EQ(Board::standard().attacks_on(16), (1ULL << 9) | (1ULL << 1));
	EXPECT_EQ(Board::standard().attacks_on(18), (1ULL << 9) | (1ULL << 1) | (1ULL << 11));
}

TEST(BoardTest, MaskIsAttacked) {
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_4, color::WHITE), false);
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_4, color::BLACK), false);

	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_1, color::WHITE), true);
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_1, color::BLACK), false);

	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_8, color::BLACK), true);
	EXPECT_EQ(Board::standard().mask_is_attacked(RANK_8, color::WHITE), false);
}

/**
 * MoveTest: Testing for move functions in move.cpp
 */

TEST(MoveTest, Construct) {
	int m = move::make(2, 4, type::QUEEN);

	EXPECT_EQ(move::src(m), 2);
	EXPECT_EQ(move::dst(m), 4);
	EXPECT_EQ(move::ptype(m), type::QUEEN);
}

TEST(MoveTest, ConstructUci) {
	EXPECT_EQ(move::from_uci("a1b1q"), move::make(0, 1, type::QUEEN));
	EXPECT_EQ(move::from_uci("a1h8n"), move::make(0, 63, type::KNIGHT));
}

TEST(MoveTest, IsValid) {
	EXPECT_TRUE(move::is_null(move::null()));
	EXPECT_FALSE(move::is_null(move::from_uci("a1b1q")));
}

/**
 * PieceTest: tests for pieces in piece.cpp
 */

TEST(PieceTest, MakePiece) {
	EXPECT_EQ(piece::make(color::WHITE, type::PAWN), 0);
	EXPECT_EQ(piece::make(color::BLACK, type::PAWN), 1);
	EXPECT_EQ(piece::make(color::WHITE, type::BISHOP), 2);
	EXPECT_EQ(piece::make(color::BLACK, type::BISHOP), 3);
	EXPECT_EQ(piece::make(color::WHITE, type::KNIGHT), 4);
	EXPECT_EQ(piece::make(color::BLACK, type::KNIGHT), 5);
	EXPECT_EQ(piece::make(color::WHITE, type::ROOK), 6);
	EXPECT_EQ(piece::make(color::BLACK, type::ROOK), 7);
	EXPECT_EQ(piece::make(color::WHITE, type::QUEEN), 8);
	EXPECT_EQ(piece::make(color::BLACK, type::QUEEN), 9);
	EXPECT_EQ(piece::make(color::WHITE, type::KING), 10);
	EXPECT_EQ(piece::make(color::BLACK, type::KING), 11);
}

TEST(PieceTest, GetColor) {
	EXPECT_EQ(piece::color(piece::make(color::WHITE, type::QUEEN)), color::WHITE);
	EXPECT_EQ(piece::color(piece::make(color::BLACK, type::KNIGHT)), color::BLACK);
}

TEST(PieceTest, GetType) {
	EXPECT_EQ(piece::type(piece::make(color::WHITE, type::QUEEN)), type::QUEEN);
	EXPECT_EQ(piece::type(piece::make(color::BLACK, type::KNIGHT)), type::KNIGHT);
}

TEST(PieceTest, IsNull) {
	EXPECT_TRUE(piece::is_null(piece::null()));
}

TEST(PieceTest, GetUci) {
	EXPECT_EQ(piece::get_uci(piece::make(color::WHITE, type::QUEEN)), 'Q');
	EXPECT_EQ(piece::get_uci(piece::make(color::BLACK, type::PAWN)), 'p');
	EXPECT_EQ(piece::get_uci(piece::make(color::WHITE, type::KNIGHT)), 'N');
	EXPECT_EQ(piece::get_uci(piece::make(color::BLACK, type::BISHOP)), 'b');
}

TEST(PieceTest, FromUci) {
	EXPECT_EQ(piece::from_uci('B'), piece::make(color::WHITE, type::BISHOP));
	EXPECT_EQ(piece::from_uci('k'), piece::make(color::BLACK, type::KING));
	EXPECT_EQ(piece::from_uci('P'), piece::make(color::WHITE, type::PAWN));
	EXPECT_EQ(piece::from_uci('q'), piece::make(color::BLACK, type::QUEEN));

	EXPECT_TRUE(piece::is_null(piece::from_uci('A')));
}

/**
 * ColorTest: tests for piece colors in color.cpp
 */

TEST(PieceTest, ColorFromUci) {
	EXPECT_EQ(color::from_uci('w'), color::WHITE);
	EXPECT_EQ(color::from_uci('b'), color::BLACK);
	EXPECT_TRUE(color::is_null(color::from_uci('g')));
}

TEST(PieceTest, ColorToUci) {
	EXPECT_EQ(color::to_uci(color::WHITE), 'w');
	EXPECT_EQ(color::to_uci(color::BLACK), 'b');
	EXPECT_EQ(color::to_uci(123), '?');
}

/**
 * TypeTest: tests for piece types in type.cpp
 */

TEST(TypeTest, TypeToUci) {
	EXPECT_EQ(type::to_uci(type::PAWN), 'p');
	EXPECT_EQ(type::to_uci(type::BISHOP), 'b');
	EXPECT_EQ(type::to_uci(type::KNIGHT), 'n');
	EXPECT_EQ(type::to_uci(type::ROOK), 'r');
	EXPECT_EQ(type::to_uci(type::QUEEN), 'q');
	EXPECT_EQ(type::to_uci(type::KING), 'k');
	EXPECT_EQ(type::to_uci(type::null()), '?');
}

TEST(TypeTest, TypeFromUci) {
	EXPECT_EQ(type::from_uci('p'), type::PAWN);
	EXPECT_EQ(type::from_uci('b'), type::BISHOP);
	EXPECT_EQ(type::from_uci('n'), type::KNIGHT);
	EXPECT_EQ(type::from_uci('r'), type::ROOK);
	EXPECT_EQ(type::from_uci('q'), type::QUEEN);
	EXPECT_EQ(type::from_uci('k'), type::KING);

	EXPECT_TRUE(type::is_null(type::from_uci('G')));
}

/**
 * PositionTest: tests for position data type in position.cpp
 */

TEST(PositionTest, CanConstruct) {
	(void) Position();
}

TEST(PositionTest, FromFen) {
	EXPECT_NO_THROW(Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")); /* standard FEN */

	EXPECT_THROW(Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 2"), std::exception); /* too many fields */
	EXPECT_THROW(Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0"), std::exception); /* too few fields */
}

TEST(PositionTest, ToFen) {
	EXPECT_EQ(Position().to_fen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

TEST(PositionTest, GetCTM) {
	EXPECT_EQ(Position().get_color_to_move(), color::WHITE);
}

TEST(PositionTest, MakeMove) {
	Position p1("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	EXPECT_TRUE(p1.make_matched_move(move::from_uci("a2a4"))); // jump
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("b4c3"))); // capture
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("e1c1"))); // qs castle
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c3b2"))); // check
	EXPECT_FALSE(p1.make_matched_move(move::from_uci("a4a5"))); // illegal push

	Position p2("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	EXPECT_TRUE(p2.make_matched_move(move::from_uci("e1g1"))); // ks castle
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c7c5"))); // jump
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("d5c6"))); // ep capture
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("e7c5"))); // quiet
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c6c7"))); // push
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("a6e2"))); // capture
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c7c8q"))); // promotion
	EXPECT_FALSE(p2.make_matched_move(move::from_uci("c5f2"))); // illegal capture
}

TEST(PositionTest, UnmakeMove) {
	Position p1("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	EXPECT_TRUE(p1.make_matched_move(move::from_uci("a2a4"))); // jump
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("b4c3"))); // capture
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("e1c1"))); // qs castle
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c3b2"))); // check
	EXPECT_FALSE(p1.make_matched_move(move::from_uci("a4a5"))); // illegal push

	for (int i = 0; i < 4; ++i) {
		EXPECT_NO_THROW(p1.unmake_move());
	}

	EXPECT_EQ(p1.to_fen(), "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	Position p2("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	EXPECT_TRUE(p2.make_matched_move(move::from_uci("e1g1"))); // ks castle
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c7c5"))); // jump
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("d5c6"))); // ep capture
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("e7c5"))); // quiet
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c6c7"))); // push
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("a6e2"))); // capture
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c7c8q"))); // promotion
	EXPECT_FALSE(p2.make_matched_move(move::from_uci("c5f2"))); // illegal capture

	for (int i = 0; i < 7; ++i) {
		EXPECT_NO_THROW(p2.unmake_move());
	}

	EXPECT_EQ(p2.to_fen(), "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
}

TEST(PositionTest, EnPassantMask) {
	Position p;
	
	EXPECT_TRUE(p.make_matched_move(move::from_uci("a2a4")));
	EXPECT_EQ(p.en_passant_mask(), bb::mask(square::A3));
}

TEST(PositionTest, GetTTKey) {
	/* Make and unmake some moves, check TT key is unchanged */
	Position p2("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	zobrist::Key init_key = p2.get_tt_key();

	EXPECT_TRUE(p2.make_matched_move(move::from_uci("e1g1"))); // ks castle
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c7c5"))); // jump
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("d5c6"))); // ep capture
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("e7c5"))); // quiet
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c6c7"))); // push
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("a6e2"))); // capture
	EXPECT_TRUE(p2.make_matched_move(move::from_uci("c7c8q"))); // promotion
	EXPECT_FALSE(p2.make_matched_move(move::from_uci("c5f2"))); // illegal capture

	for (int i = 0; i < 7; ++i) {
		EXPECT_NO_THROW(p2.unmake_move());
	}

	EXPECT_EQ(p2.get_tt_key(), init_key);
}

TEST(PositionTest, Check) {
	Position p1("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	EXPECT_TRUE(p1.make_matched_move(move::from_uci("a2a4"))); // jump
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("b4c3"))); // capture
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("e1c1"))); // qs castle
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c3b2"))); // check

	EXPECT_TRUE(p1.check());
}

TEST(PositionTest, NumRepetitions) {
	Position p1("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c3a4")));
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("b6c8")));
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("a4c3")));
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c8b6")));

	EXPECT_EQ(p1.num_repetitions(), 2);
}

TEST(PositionTest, HalfmoveClock) {
	/* Start with nonzero HM clock */
	Position p1("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 3 1");

	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c3a4")));
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("b6c8")));
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("a4c3")));
	EXPECT_TRUE(p1.make_matched_move(move::from_uci("c8b6")));

	EXPECT_EQ(p1.halfmove_clock(), 7);

	EXPECT_TRUE(p1.make_matched_move(move::from_uci("e5f7"))); // capture, reset clock

	EXPECT_EQ(p1.halfmove_clock(), 0);
}

TEST(PositionTest, PseudolegalMoves) {
	/* Test a position with every type of move available! */
	Position p("rnbqkbr1/1P2ppp1/5n1p/p1pP4/p1B5/N4N2/1BPPQPPP/R3K2R w KQq c6 0 13");

	int moves[MAX_PL_MOVES];
	int move_count;

	move_count = p.pseudolegal_moves(moves);

	EXPECT_EQ(move_count, 52);

	auto contains = [&](int m) -> bool {
		for (int i = 0; i < move_count; ++i) {
			if (move::match(moves[i], m)) {
				return true;
			}
		}

		return false;
	};

	EXPECT_TRUE(contains(move::from_uci("d2d4"))); // pawn jumps
	EXPECT_TRUE(contains(move::from_uci("g2g4")));
	EXPECT_TRUE(contains(move::from_uci("h2h4")));

	EXPECT_TRUE(contains(move::from_uci("d5d6"))); // pushes
	EXPECT_TRUE(contains(move::from_uci("c2c3")));
	EXPECT_TRUE(contains(move::from_uci("d2d3")));
	EXPECT_TRUE(contains(move::from_uci("g2g3")));
	EXPECT_TRUE(contains(move::from_uci("h2h3")));

	EXPECT_TRUE(contains(move::from_uci("b2c1"))); // bishop quiets
	EXPECT_TRUE(contains(move::from_uci("b2c3")));
	EXPECT_TRUE(contains(move::from_uci("b2d4")));
	EXPECT_TRUE(contains(move::from_uci("b2e5")));
	EXPECT_TRUE(contains(move::from_uci("c4b3")));
	EXPECT_TRUE(contains(move::from_uci("c4a2")));
	EXPECT_TRUE(contains(move::from_uci("c4b5")));
	EXPECT_TRUE(contains(move::from_uci("c4a6")));
	EXPECT_TRUE(contains(move::from_uci("c4d3")));

	EXPECT_TRUE(contains(move::from_uci("a3b5"))); // knight quiets
	EXPECT_TRUE(contains(move::from_uci("a3b1")));
	EXPECT_TRUE(contains(move::from_uci("f3d4")));
	EXPECT_TRUE(contains(move::from_uci("f3e5")));
	EXPECT_TRUE(contains(move::from_uci("f3g5")));
	EXPECT_TRUE(contains(move::from_uci("f3h4")));
	EXPECT_TRUE(contains(move::from_uci("f3g1")));

	EXPECT_TRUE(contains(move::from_uci("a1b1"))); // rook quiets
	EXPECT_TRUE(contains(move::from_uci("a1c1")));
	EXPECT_TRUE(contains(move::from_uci("a1d1")));
	EXPECT_TRUE(contains(move::from_uci("a1a2")));
	EXPECT_TRUE(contains(move::from_uci("h1g1")));
	EXPECT_TRUE(contains(move::from_uci("h1f1")));

	EXPECT_TRUE(contains(move::from_uci("e2d1"))); // queen quiets
	EXPECT_TRUE(contains(move::from_uci("e2f1")));
	EXPECT_TRUE(contains(move::from_uci("e2d3")));
	EXPECT_TRUE(contains(move::from_uci("e2e3")));
	EXPECT_TRUE(contains(move::from_uci("e2e4")));
	EXPECT_TRUE(contains(move::from_uci("e2e5")));
	EXPECT_TRUE(contains(move::from_uci("e2e6")));

	EXPECT_TRUE(contains(move::from_uci("e2e7"))); // queen captures
	EXPECT_TRUE(contains(move::from_uci("b2f6"))); // bishop captures

	EXPECT_TRUE(contains(move::from_uci("b7a8q"))); // promoting left captures
	EXPECT_TRUE(contains(move::from_uci("b7a8n")));
	EXPECT_TRUE(contains(move::from_uci("b7a8b")));
	EXPECT_TRUE(contains(move::from_uci("b7a8r")));

	EXPECT_TRUE(contains(move::from_uci("b7c8q"))); // promoting right captures
	EXPECT_TRUE(contains(move::from_uci("b7c8n")));
	EXPECT_TRUE(contains(move::from_uci("b7c8b")));
	EXPECT_TRUE(contains(move::from_uci("b7c8r")));

	EXPECT_TRUE(contains(move::from_uci("d5c6"))); // en-passant

	EXPECT_TRUE(contains(move::from_uci("e1g1"))); // KS castle
	EXPECT_TRUE(contains(move::from_uci("e1c1"))); // QS castle
}

TEST(PositionTest, PseudolegalMovesEvasions) {
	/* Test a position with every type of move available! */
	Position p("r1b1kbnr/ppp1ppp1/2B4p/q7/8/2N2N2/PPPP1PPP/R1BQK2R b KQkq - 0 6");

	int moves[MAX_PL_MOVES];
	int move_count;

	move_count = p.pseudolegal_moves_evasions(moves);

	EXPECT_EQ(move_count, 4);

	auto contains = [&](int m) -> bool {
		for (int i = 0; i < move_count; ++i) {
			if (move::match(moves[i], m)) {
				return true;
			}
		}

		return false;
	};

	EXPECT_TRUE(contains(move::from_uci("c8d7"))); // bishop blocks
	EXPECT_TRUE(contains(move::from_uci("b7c6"))); // pawn captures attacker

	EXPECT_TRUE(contains(move::from_uci("e8d8"))); // quiet king moves

	EXPECT_TRUE(contains(move::from_uci("e8d7"))); // another quiet king move-- this is illegal, but OK in the qmovegen
}

TEST(PositionTest, MakeUnmakeConsistency) {
	// Test consistency of make/unmake move with a variety of moves
	Position p("rnbq2r1/4b2P/p4p2/Pp1k4/2p1NQ1N/2B1P3/1PP2PP1/R3K2R w KQ b6 0 26");

	// The above position contains several different types of moves for white:
	//
	// h8=? promoting advance
	// hxg8=? promoting captures
	// hxg8=B,Q promoting captures with check
	// g4 pawn jump
	// O-O-O castling with check
	// O-O castling without check
	// axb6 en-passant
	// Nxf6 standard capture with check
	// Qf5 quiet check

	int moves[MAX_PL_MOVES];
	int move_count;

	move_count = p.pseudolegal_moves(moves);

	EXPECT_EQ(move_count, 56);

	for (int i = 0; i < move_count; ++i) {
		std::string f1 = p.to_fen();

		EXPECT_TRUE(p.make_move(moves[i]));
		EXPECT_EQ(p.unmake_move(), moves[i]);

		std::string f2 = p.to_fen();

		// FEN should be unchanged after making and unmaking move.
		EXPECT_EQ(f1, f2);
	}
}

/**
 * PerftTest: movegen perft testing
 */
TEST(PerftTest, StandardPerft) {
	Position p;
	perft::results res;

	res = perft::run(p, 0);

	EXPECT_EQ(res.nodes, 1);
	EXPECT_EQ(res.captures, 0);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles , 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 0);

	res = perft::run(p, 1);

	EXPECT_EQ(res.nodes, 20);
	EXPECT_EQ(res.captures, 0);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 0);

	res = perft::run(p, 2);

	EXPECT_EQ(res.nodes, 400);
	EXPECT_EQ(res.captures, 0);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 0);

	res = perft::run(p, 3);

	EXPECT_EQ(res.nodes, 8902);
	EXPECT_EQ(res.captures, 34);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 12);

	res = perft::run(p, 4);

	EXPECT_EQ(res.nodes, 197281);
	EXPECT_EQ(res.captures, 1576);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 469);

	res = perft::run(p, 5);

	EXPECT_EQ(res.nodes, 4865609);
	EXPECT_EQ(res.captures, 82719);
	EXPECT_EQ(res.en_passant, 258);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 27351);
}

TEST(PerftTest, PerftKiwipete) {
	Position p("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	perft::results res;

	res = perft::run(p, 1);

	EXPECT_EQ(res.nodes, 48);
	EXPECT_EQ(res.captures, 8);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 2);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 0);

	res = perft::run(p, 2);

	EXPECT_EQ(res.nodes, 2039);
	EXPECT_EQ(res.captures, 351);
	EXPECT_EQ(res.en_passant, 1);
	EXPECT_EQ(res.castles, 91);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 3);

	res = perft::run(p, 3);

	EXPECT_EQ(res.nodes, 97862);
	EXPECT_EQ(res.captures, 17102);
	EXPECT_EQ(res.en_passant, 45);
	EXPECT_EQ(res.castles, 3162);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 993);

	res = perft::run(p, 4);

	EXPECT_EQ(res.nodes, 4085603);
	EXPECT_EQ(res.captures, 757163);
	EXPECT_EQ(res.en_passant, 1929);
	EXPECT_EQ(res.castles, 128013);
	EXPECT_EQ(res.promotions, 15172);
	EXPECT_EQ(res.checks, 25523);
}

TEST(PerftTest, PerftThree) {
	Position p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
	perft::results res;

	res = perft::run(p, 1);

	EXPECT_EQ(res.nodes, 14);
	EXPECT_EQ(res.captures, 1);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 2);

	res = perft::run(p, 2);

	EXPECT_EQ(res.nodes, 191);
	EXPECT_EQ(res.captures, 14);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 10);

	res = perft::run(p, 3);

	EXPECT_EQ(res.nodes, 2812);
	EXPECT_EQ(res.captures, 209);
	EXPECT_EQ(res.en_passant, 2);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 267);

	res = perft::run(p, 4);

	EXPECT_EQ(res.nodes, 43238);
	EXPECT_EQ(res.captures, 3348);
	EXPECT_EQ(res.en_passant, 123);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 1680);

	res = perft::run(p, 5);

	EXPECT_EQ(res.nodes, 674624);
	EXPECT_EQ(res.captures, 52051);
	EXPECT_EQ(res.en_passant, 1165);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 52950);
}

TEST(PerftTest, PerftFour) {
	Position p("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
	perft::results res;

	res = perft::run(p, 1);

	EXPECT_EQ(res.nodes, 6);
	EXPECT_EQ(res.captures, 0);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 0);
	EXPECT_EQ(res.checks, 0);

	res = perft::run(p, 2);

	EXPECT_EQ(res.nodes, 264);
	EXPECT_EQ(res.captures, 87);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 6);
	EXPECT_EQ(res.promotions, 48);
	EXPECT_EQ(res.checks, 10);

	res = perft::run(p, 3);

	EXPECT_EQ(res.nodes, 9467);
	EXPECT_EQ(res.captures, 1021);
	EXPECT_EQ(res.en_passant, 4);
	EXPECT_EQ(res.castles, 0);
	EXPECT_EQ(res.promotions, 120);
	EXPECT_EQ(res.checks, 38);

	res = perft::run(p, 4);

	EXPECT_EQ(res.nodes, 422333);
	EXPECT_EQ(res.captures, 131393);
	EXPECT_EQ(res.en_passant, 0);
	EXPECT_EQ(res.castles, 7795);
	EXPECT_EQ(res.promotions, 60032);
	EXPECT_EQ(res.checks, 15492);
}

TEST(PerftTest, PerftFive) {
	Position p("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

	perft::results res = perft::run(p, 1);

	EXPECT_EQ(res.nodes, 44);

	res = perft::run(p, 2);

	EXPECT_EQ(res.nodes, 1486);

	res = perft::run(p, 3);

	EXPECT_EQ(res.nodes, 62379);

	res = perft::run(p, 4);

	EXPECT_EQ(res.nodes, 2103487);
}

TEST(PerftTest, PerftSix) {
	Position p("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
	perft::results res;

	res = perft::run(p, 1);
	EXPECT_EQ(res.nodes, 46);

	res = perft::run(p, 2);
	EXPECT_EQ(res.nodes, 2079);

	res = perft::run(p, 3);
	EXPECT_EQ(res.nodes, 89890);

	res = perft::run(p, 4);
	EXPECT_EQ(res.nodes, 3894594);
}

/* LogTest: basic tests for logging functions */

TEST(LogTest, SetColor) {
	log::set_color(log::ColorMode::IF_TTY);

	EXPECT_EQ(log::get_color(), log::ColorMode::IF_TTY);
}

TEST(LogTest, SetLevel) {
	log::set_level(log::DEBUG);

	EXPECT_EQ(log::get_level(), log::DEBUG);
}

/* Testing entry point */

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	bb::init();
	attacks::init();
	zobrist::init();

	return RUN_ALL_TESTS();
}
