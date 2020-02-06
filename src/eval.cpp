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

float eval::phase(u8* board) {
    /* compute non-pawn material, find difference */
    float npm = 0.0f;

    for (u8 s = 0; s < 64; ++s) {
        u8 p = board[s];
        if (!piece::exists(p)) continue;
        if (piece::type(p) == piece::Type::PAWN) continue;

        npm += _nc2_eval_type_values[piece::type(p)];
    }

    return 1.0f - (npm / _nc2_eval_total_npm);
}

float eval::evaluate(u8* board, u64 white_attacks, u64 black_attacks) {
    float p = eval::phase(board);
    float score = 0.0f;

    for (u8 s = 0; s < 64; ++s) {
        score += p * _nc2_eval_endgame_scores[s][board[s]];
        score += (1.0f - p) * _nc2_eval_opening_scores[s][board[s]];
    }

    score += eval::center_control(white_attacks);
    score -= eval::center_control(black_attacks);

    return score;
}
