#pragma once

/*
 * Heuristic evaluation functions
 */

#include "types.h"
#include "piece.h"

namespace nc2 {
    namespace eval {
        float development(u8* board, u8 col);
        float center_control(u64 attack_mask);
        float material_diff(u8* board);
        float advanced_pawns(u8* board, u8 col);
        float phase(u8* board);

        constexpr float TEMPO_VALUE = 0.15f;
        constexpr float CENTER_CONTROL_VALUE = 1.0f;
        constexpr float DEVELOPMENT_VALUE = 0.75f;
        constexpr float ADV_PAWN_VALUE = 0.1f;
    }
}
