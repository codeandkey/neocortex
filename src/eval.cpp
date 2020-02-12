#include "eval.h"
#include "square.h"

using namespace nc2;

static const u64 _nc2_eval_center_mask = nc2::square::MASK_D4 | nc2::square::MASK_E4 | nc2::square::MASK_E5 | nc2::square::MASK_D5;

static float _nc2_eval_opening_scores[64][16] = { 0.0f };
static float _nc2_eval_endgame_scores[64][16] = { 0.0f };

static const float _nc2_eval_type_values[] = {
    1.0f, /* pawn */
    3.0f, /* knight */
    3.2f, /* bishop */
    5.0f, /* rook */
    0.0f, /* king */
    7.0f, /* queen */
};

static float _nc2_eval_npm_values[] = {
    0.0f,
    0.0f,
    _nc2_eval_type_values[piece::Type::KNIGHT],
    _nc2_eval_type_values[piece::Type::KNIGHT],
    _nc2_eval_type_values[piece::Type::BISHOP],
    _nc2_eval_type_values[piece::Type::BISHOP],
    _nc2_eval_type_values[piece::Type::ROOK],
    _nc2_eval_type_values[piece::Type::ROOK],
    0.0f,
    0.0f,
    _nc2_eval_type_values[piece::Type::QUEEN],
    _nc2_eval_type_values[piece::Type::QUEEN],
    0.0f,
    0.0f,
    0.0f,
    0.0f,
};

static const float _nc2_eval_total_npm = 4 * _nc2_eval_type_values[piece::Type::KNIGHT] + \
                                         4 * _nc2_eval_type_values[piece::Type::ROOK] + \
                                         4 * _nc2_eval_type_values[piece::Type::BISHOP] + \
                                         2 * _nc2_eval_type_values[piece::Type::QUEEN];

void eval::init() {
    /* Initialize score tables. */

    /* Add material values to both tables. */
    for (u8 s = 0; s < 64; ++s) {
        for (u8 p = 0; p < 12; ++p) {
            if (piece::color(p) == piece::Color::WHITE) {
                _nc2_eval_opening_scores[s][p] += _nc2_eval_type_values[piece::type(p)];
                _nc2_eval_endgame_scores[s][p] += _nc2_eval_type_values[piece::type(p)];
            } else {
                _nc2_eval_opening_scores[s][p] -= _nc2_eval_type_values[piece::type(p)];
                _nc2_eval_endgame_scores[s][p] -= _nc2_eval_type_values[piece::type(p)];
            }
        }
    }

    /* Add development value to opening table. */
    for (u8 f = 0; f < 8; ++f) {
        _nc2_eval_opening_scores[square::at(2, f)][piece::WHITE_KNIGHT] += DEVELOPMENT_VALUE;
        _nc2_eval_opening_scores[square::at(2, f)][piece::WHITE_BISHOP] += DEVELOPMENT_VALUE;

        _nc2_eval_opening_scores[square::at(5, f)][piece::BLACK_KNIGHT] -= DEVELOPMENT_VALUE;
        _nc2_eval_opening_scores[square::at(5, f)][piece::BLACK_BISHOP] -= DEVELOPMENT_VALUE;
    }

    /* Add advanced pawn value to endgame table. */
    for (u8 r = 2; r < 7; ++r) {
        for (u8 f = 0; f < 8; ++f) {
            _nc2_eval_endgame_scores[square::at(r, f)][piece::WHITE_PAWN] += ADV_PAWN_VALUE * (r - 1);
        }
    }

    for (u8 r = 5; r > 0; --r) {
        for (u8 f = 0; f < 8; ++f) {
            _nc2_eval_endgame_scores[square::at(r, f)][piece::BLACK_PAWN] -= ADV_PAWN_VALUE * (6 - r);
        }
    }
}

float eval::center_control(u64 attack_mask) {
    int ct = __builtin_popcount(attack_mask & _nc2_eval_center_mask);
    return CENTER_CONTROL_VALUE * ct;
}

float eval::king_safety(u64 king_attacks, u64 self_attacks) {
    return KING_SAFETY_VALUE * __builtin_popcount(king_attacks & self_attacks);
}

float eval::king_attacks(u64 other_king_attacks, u64 attack_mask) {
    return KING_ATTACK_VALUE * __builtin_popcount(other_king_attacks & attack_mask);
}

float eval::evaluate(u8* board, u64 white_attacks, u64 black_attacks, u64 white_king_attacks, u64 black_king_attacks) {
    float score = 0.0f;

    float npm = 0.0f;
    float phase;
    float endgame_score = 0.0f, opening_score = 0.0f;

    for (u8 s = 0; s < 64; ++s) {
        u8 p = board[s];

        npm += _nc2_eval_npm_values[p];
        endgame_score += _nc2_eval_endgame_scores[s][p];
        opening_score += _nc2_eval_opening_scores[s][p];
    }

    phase = (npm / _nc2_eval_total_npm);
    score += phase * opening_score;
    score += (1.0f - phase) * endgame_score;

    score += eval::center_control(white_attacks);
    score -= eval::center_control(black_attacks);

    score += eval::king_safety(white_king_attacks, white_attacks);
    score -= eval::king_safety(black_king_attacks, black_attacks);

    score += eval::king_attacks(black_king_attacks, white_attacks);
    score -= eval::king_attacks(white_king_attacks, black_attacks);

    return score;
}

float eval::noise() {
    return (((rand() % 100) / 100.0f) * 2.0f - 1.0f) * NOISE_THRESHOLD;
}


