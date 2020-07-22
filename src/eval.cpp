#include "eval.h"
#include "eval_consts.h"
#include "util.h"

using namespace pine;

Score::Score(int value) : value(value) {}

bool Score::is_mate() {
	return (value >= CHECKMATE - MATE_THRESHOLD || value <= CHECKMATED + MATE_THRESHOLD);
}

Score Score::parent() {
	if (is_mate()) {
		if (value < 0) return value + 1;
		if (value > 0) return value - 1;
	} else {
		return *this;
	}
}

std::string Score::to_string() {
	if (is_mate()) {
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

std::string Score::to_uci() {
	if (is_mate()) {
		int dist = 0;

		if (value > 0) {
			dist = CHECKMATE - value;
		}
		else {
			dist = -(value - CHECKMATED);
		}

		return util::format("mate %d", dist);
	} else {
		return util::format("%d", value);
	}
}

Score Score::operator-() {
	return Score(-value);
}

Eval::Eval(Position& pos) : pos(pos) {
	/* Compute material values */
	Board& b = pos.get_board();

	material_mg[piece::WHITE] = material_mg[piece::BLACK] = 0;
	material_eg[piece::WHITE] = material_eg[piece::BLACK] = 0;

	for (int t = 0; t < 6; ++t) {
		int count = bb::popcount(b.get_color_occ(piece::WHITE) & b.get_piece_occ(t));

		material_mg[piece::WHITE] += count * eval::MATERIAL_MG[t];
		material_mg[piece::BLACK] += count * eval::MATERIAL_MG[t];
		material_eg[piece::WHITE] += count * eval::MATERIAL_EG[t];
		material_eg[piece::BLACK] += count * eval::MATERIAL_EG[t];

		if (t != piece::PAWN) {
			material_nonpawn_mg += count * eval::MATERIAL_MG[t];
		}
	}

	phase = (material_nonpawn_mg * 256) / eval::MATERIAL_MG_MAX;
}

Score Eval::to_score() {
	int score = 0;

	int material_score_mg = material_mg[pos.get_color_to_move()] - material_mg[!pos.get_color_to_move()];
	int material_score_eg = material_eg[pos.get_color_to_move()] - material_eg[pos.get_color_to_move()];

	score += (material_score_mg * phase) / 255;
	score += (material_score_eg * phase) / 255;
	
	return score;
}