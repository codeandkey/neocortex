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
         * Evaluates king safety at a position.
         * Considers squares around the king protected and attacked.
         *
         * @param king_attacks King attack mask
         * @param self_attacks Self attacks (same color as king)
         */
        float king_safety(u64 king_attacks, u64 self_attacks);

        /**
         * Evaluates king attacks at a position.
         *
         * @param king_attacks Other king attack mask
         * @param self_attacks Self attack
         */
        float king_attacks(u64 king_attacks, u64 self_attacks);

        /**
         * Evaluates a position.
         *
         * @param board Board state.
         * @param white_attacks White attack mask.
         * @param black_attacks Black attack mask.
         * @param white_king_attacks White king attack mask.
         * @param black_king_attacks Black king attack mask.
         *
         * @return Heuristic position evaluation.
         */
        float evaluate(u8* board, u64 white_attacks, u64 black_attacks, u64 white_king_attacks, u64 black_king_attacks);

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
        constexpr float KING_SAFETY_VALUE = 0.3f; /* value safety equal to attacks.. for now */
        constexpr float KING_ATTACK_VALUE = 0.3f;
    }
}
