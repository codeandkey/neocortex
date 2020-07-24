#include "attacks.h"
#include "eval.h"
#include "eval_consts.h"
#include "util.h"

using namespace pine;

bool score::is_mate(int value) {
	return (value >= CHECKMATE - MATE_THRESHOLD || value <= CHECKMATED + MATE_THRESHOLD);
}

int score::parent(int value) {
	if (value == INCOMPLETE) {
		return value;
	}

	if (is_mate(value)) {
		if (value < 0) return value + 1;
		if (value > 0) return value - 1;

		return 0;
	} else {
		return value;
	}
}

std::string score::to_string(int value) {
	if (is_mate(value)) {
		int dist = 0;

		if (value > 0) {
			dist = CHECKMATE - value;
		}
		else {
			dist = -(value - CHECKMATED);
		}

		return util::format("#%d", dist);
	} else {
		return util::format("%+.2f", value / 100.0f);
	}
}

std::string score::to_uci(int value) {
	if (is_mate(value)) {
		int dist = 0;

		if (value > 0) {
			dist = CHECKMATE - value;
		}
		else {
			dist = -(value - CHECKMATED);
		}

		return util::format("mate %d", dist);
	} else {
		return util::format("cp %d", value);
	}
}

Eval::Eval(Position& pos) : pos(pos) {
	/* Compute material values */
	Board& b = pos.get_board();

	material_mg[piece::WHITE] = material_mg[piece::BLACK] = 0;
	material_eg[piece::WHITE] = material_eg[piece::BLACK] = 0;

	for (int t = 0; t < 6; ++t) {
		int white = bb::popcount(b.get_color_occ(piece::WHITE) & b.get_piece_occ(t));
		int black = bb::popcount(b.get_color_occ(piece::BLACK) & b.get_piece_occ(t));

		material_mg[piece::WHITE] += white * eval::MATERIAL_MG[t];
		material_mg[piece::BLACK] += black * eval::MATERIAL_MG[t];
		material_eg[piece::WHITE] += white * eval::MATERIAL_EG[t];
		material_eg[piece::BLACK] += black * eval::MATERIAL_EG[t];
	}

	phase = ((material_mg[piece::WHITE] + material_mg[piece::BLACK]) * 256) / eval::MATERIAL_MG_MAX;

	/* Compute attack mobility bonus */
	attackbonus[piece::WHITE] = bb::popcount(pos.get_current_attacks(piece::WHITE)) * eval::ATTACK_BONUS;
	attackbonus[piece::BLACK] = bb::popcount(pos.get_current_attacks(piece::BLACK)) * eval::ATTACK_BONUS;

	/* Compute safe mobility bonus */
	for (int c = 0; c < 2; ++c) {
		mobility_mg[c] = mobility_eg[c] = 0;

		int bishop_mobility = 0, knight_mobility = 0, rook_mobility = 0, queen_mobility = 0;

		bitboard bishops = pos.get_board().get_color_occ(c) & pos.get_board().get_piece_occ(piece::BISHOP);

		while (bishops) {
			bishop_mobility += bb::popcount(~pos.get_current_attacks(!c) & attacks::bishop(bb::poplsb(bishops), pos.get_board().get_global_occ()));
		}

		bitboard knights = pos.get_board().get_color_occ(c) & pos.get_board().get_piece_occ(piece::KNIGHT);

		while (knights) {
			knight_mobility += bb::popcount(~pos.get_current_attacks(!c) & attacks::knight(bb::poplsb(knights)));
		}

		bitboard rooks = pos.get_board().get_color_occ(c) & pos.get_board().get_piece_occ(piece::ROOK);

		while (rooks) {
			rook_mobility += bb::popcount(~pos.get_current_attacks(!c) & attacks::rook(bb::poplsb(rooks), pos.get_board().get_global_occ()));
		}

		bitboard queens = pos.get_board().get_color_occ(c) & pos.get_board().get_piece_occ(piece::QUEEN);

		while (queens) {
			queen_mobility += bb::popcount(~pos.get_current_attacks(!c) & attacks::queen(bb::poplsb(queens), pos.get_board().get_global_occ()));
		}

		mobility_mg[c] += bishop_mobility * eval::MOBILITY_BISHOP;
		mobility_mg[c] += knight_mobility * eval::MOBILITY_KNIGHT;
		mobility_mg[c] += rook_mobility * eval::MOBILITY_ROOK_MG;
		mobility_mg[c] += queen_mobility * eval::MOBILITY_QUEEN;

		mobility_eg[c] += bishop_mobility * eval::MOBILITY_BISHOP;
		mobility_eg[c] += knight_mobility * eval::MOBILITY_KNIGHT;
		mobility_eg[c] += rook_mobility * eval::MOBILITY_ROOK_EG;
		mobility_eg[c] += queen_mobility * eval::MOBILITY_QUEEN;
	}

	center_control[piece::WHITE] = center_control[piece::BLACK] = 0;

	/* Compute center square control */
	int center_squares[4] = { 27, 28, 35, 36 };
	for (int i = 0; i < 4; ++i) {
		int white, black;
		pos.get_attackers_defenders(center_squares[i], white, black);

		center_control[piece::WHITE] += white * eval::CENTER_CONTROL;
		center_control[piece::BLACK] += black * eval::CENTER_CONTROL;
	}

	/* Compute king safety */
	king_safety[piece::WHITE] = king_safety[piece::BLACK] = 0;

	for (int c = 0; c < 2; ++c) {
		int ksq = bb::getlsb(pos.get_board().get_color_occ(c) & pos.get_board().get_piece_occ(piece::KING));
		bitboard king_squares = attacks::king(ksq);

		while (king_squares) {
			int next = bb::poplsb(king_squares);

			/* Get AD info on king squares */
			int white = 0, black = 0;
			pos.get_attackers_defenders(next, white, black);

			int attack_colors[2] = { white, black };

			/*if (attack_colors[c] < attack_colors[!c]) {
				king_safety[c] -= eval::KING_SAFETY;
			}
			else if (attack_colors[c] > attack_colors[!c]) {
				king_safety[c] += eval::KING_SAFETY;
			}*/

			/* this could be better, but could also incentivise severely overprotecting the king */
			king_safety[c] += (attack_colors[c] - attack_colors[!c]) * eval::KING_SAFETY;
		}
	}

	/* Compute blocking pawn penalties, passed pawn bonus */
	blocking_pawns[piece::WHITE] = blocking_pawns[piece::BLACK] = 0;
	passed_pawns[piece::WHITE] = passed_pawns[piece::BLACK] = 0;

	for (int c = 0; c < 2; ++c) {
		for (int f = 0; f < 8; ++f) {
			int count = bb::popcount(bb::file(f) & pos.get_board().get_color_occ(c) & pos.get_board().get_piece_occ(piece::PAWN));

			if (count > 1) {
				blocking_pawns[c] += eval::BLOCKING_PAWNS;

				if (!bb::popcount(bb::file(f) & pos.get_board().get_color_occ(!c) & pos.get_board().get_piece_occ(piece::PAWN))) {
					passed_pawns[c] += eval::PASSED_PAWNS;
				}
			}
		}
	}
}

int Eval::to_score() {
	int score = 0;

	int material_score_mg = material_mg[pos.get_color_to_move()] - material_mg[!pos.get_color_to_move()];
	int material_score_eg = material_eg[pos.get_color_to_move()] - material_eg[!pos.get_color_to_move()];

	score += (material_score_mg * phase) / 256;
	score += (material_score_eg * (256 - phase)) / 256;

	score += attackbonus[pos.get_color_to_move()] - attackbonus[!pos.get_color_to_move()];

	int mobility_score_mg = mobility_mg[pos.get_color_to_move()] - mobility_mg[!pos.get_color_to_move()];
	int mobility_score_eg = mobility_eg[pos.get_color_to_move()] - mobility_eg[!pos.get_color_to_move()];

	score += (mobility_score_mg * phase) / 256;
	score += (mobility_score_eg * (256 - phase)) / 256;

	score +=  (phase * (center_control[pos.get_color_to_move()] - center_control[!pos.get_color_to_move()])) / 256;

	score += (king_safety[pos.get_color_to_move()] - king_safety[pos.get_color_to_move()]);

	score += (blocking_pawns[pos.get_color_to_move()] - blocking_pawns[pos.get_color_to_move()]);
	
	return score + eval::TEMPO_BONUS;
}

std::string Eval::to_table() {
	std::string output;
	
	output +=              "            | white | black |\n";
	output +=              "------------|-------|-------|\n";
	output += util::format("material_mg | %5d | %5d |\n", material_mg[piece::WHITE], material_mg[piece::BLACK]);
	output += util::format("material_eg | %5d | %5d |\n", material_eg[piece::WHITE], material_eg[piece::BLACK]);
	output += util::format("attackbonus | %5d | %5d |\n", attackbonus[piece::WHITE], attackbonus[piece::BLACK]);
	output += util::format("mobility_mg | %5d | %5d |\n", mobility_mg[piece::WHITE], mobility_mg[piece::BLACK]);
	output += util::format("mobility_eg | %5d | %5d |\n", mobility_eg[piece::WHITE], mobility_eg[piece::BLACK]);
	output += util::format("ctr_control | %5d | %5d |\n", center_control[piece::WHITE], center_control[piece::BLACK]);
	output += util::format("king_safety | %5d | %5d |\n", king_safety[piece::WHITE], king_safety[piece::BLACK]);
	output += util::format("blocked_pns | %5d | %5d |\n", blocking_pawns[piece::WHITE], blocking_pawns[piece::BLACK]);
	output += util::format("passed_pns  | %5d | %5d |\n", passed_pawns[piece::WHITE], passed_pawns[piece::BLACK]);
	output += util::format("phase       | %13d |\n", phase);

	return output;
}