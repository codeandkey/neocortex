#include "eval.h"
#include "square.h"

using namespace nc2;

static const u64 _nc2_eval_center_mask = nc2::square::MASK_D4 | nc2::square::MASK_E4 | nc2::square::MASK_E5 | nc2::square::MASK_D5;

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

float eval::development(u8* board, u8 col) {
    u8 start = 2 * 8, count = 0;
    if (col == piece::Color::BLACK) start = 5 * 8;

    for (u8 f = 0; f < 8; ++f) {
        u8 p = board[start + f];

        if (piece::color(p) == col) {
            switch (piece::type(p)) {
                case piece::Type::BISHOP:
                case piece::Type::KNIGHT:
                    ++count;
                default:
                    break;
            }
        }
    }

    return DEVELOPMENT_VALUE * count;
}

float eval::center_control(u64 attack_mask) {
    int ct = __builtin_popcount(attack_mask & _nc2_eval_center_mask);
    return CENTER_CONTROL_VALUE * ct;
}

float eval::material_diff(u8* board) {
    float out = 0.0f;

    for (u8 s = 0; s < 64; ++s) {
        u8 p = board[s];

        if (piece::exists(p)) {
            if (piece::color(p) == piece::Color::WHITE) {
                out += _nc2_eval_type_values[piece::type(p)];
            } else {
                out -= _nc2_eval_type_values[piece::type(p)];
            }
        }
    }

    return out;
}

float eval::advanced_pawns(u8* board, u8 col) {
    int score = 0;

    if (col == piece::Color::WHITE) {
        for (u8 s = 0; s < 64; ++s) {
            if (board[s] == piece::WHITE_PAWN) {
                score += square::rank(s) - 1;
            }
        }
    } else if (col == piece::Color::BLACK) {
        for (u8 s = 0; s < 64; ++s) {
            if (board[s] == piece::BLACK_PAWN) {
                score += 7 - square::rank(s);
            }
        }
    }

    return score * ADV_PAWN_VALUE;
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
