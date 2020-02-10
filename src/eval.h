#pragma once

/*
 * Heuristic evaluation functions
 */

#include "types.h"
#include "piece.h"

namespace nc2 {
    namespace eval {
        /**
         * Initializes the eval score tables.
         * Must be called before any other eval calls.
         */
        void init();

        /**
         * Gets the center control score for a given attack mask.
         * Always positive.
         *
         * @param attack_mask Mask to score.
         * @return Center control score.
         */
        float center_control(u64 attack_mask);

        /**
         * Evaluates a position.
         *
         * @param board Board state.
         * @param white_attacks White attack mask.
         * @param black_attacks Black attack mask.
         *
         * @return Heuristic position evaluation.
         */
        float evaluate(u8* board, u64 white_attacks, u64 black_attacks);

        /**
         * Gets noise which can be added to a heuristic. May be positive or negative.
         * Amplitude is determined by NOISE_THRESHOLD.
         *
         * @return Evaluation noise.
         */
        float noise();

        constexpr float TEMPO_VALUE = 0.15f;
        constexpr float CENTER_CONTROL_VALUE = 1.0f;
        constexpr float DEVELOPMENT_VALUE = 0.95f;
        constexpr float ADV_PAWN_VALUE = 0.5f;
        constexpr float NOISE_THRESHOLD = 0.01f;
    }
}
