#pragma once

#include <string>

namespace nc2 {
    class Evaluation {
        public:
            /**
             * Initializes an evaluation.
             *
             * @param eval Evaluation value.
             * @param has_mate Should be true if there is mate on the board.
             * @param mate_in Number of moves until mate. Negative for black.
             */
            Evaluation(float eval = 0.0f, bool has_mate = false, int mate_in = 0);

            /**
             * Returns the evaluation value.
             * If there is forced mate, this function returns 0.0f.
             *
             * Negative values imply a better position for black, and positive
             * values imply a better position for white.
             *
             * @return Evaluation value, or 0.0f.
             */
            float get_eval();

            /**
             * Gets if the evalutaion is a forced win.
             *
             * @return true if there is forced mate on the board, false otherwise
             */
            bool get_forced_mate();

            /**
             * Gets the moves until a forced mate.
             * A negative value implies forced mate for black, and positive for white.
             * If there is no forced mate, this function returns 0.
             *
             * @return Forced mate counter.
             */
            int get_mate_in();

            /**
             * Converts the evaluation to a string.
             *
             * @return Evaluation in string format.
             */
            std::string to_string();

            /**
             * Converts the evaluation to a UCI "score" string.
             *
             * @return Evaluation in string format.
             */
            std::string to_uci_string();

            /**
             * Shorthand for to_string().
             */
            operator std::string();

            /**
             * Compares the evaluation with another.
             * There need to be 3 cases (white, black, draw) so this is not
             * implemented with 'operator <'.
             *
             * @param b Other evaluation.
             * @return -1 if b is better for black, 1 if b is better for white, 0 if this eval is equal to b.
             */
            int compare(const Evaluation& b) const;
            
            /**
             * Adds a random amount of noise to the evaluation.
             *
             * @param amp Maximum amplitude in both directions
             */
            void add_noise(float amp);

            /**
             * Sets the mate in counter.
             *
             * @param m Number of moves until mate.
             */
            void set_mate_in(int m);

            /**
             * Compares the evaluation with another.
             *
             * @param rhs Right-hand side.
             * @return true if this < `rhs`.
             */
            bool operator < (const Evaluation& rhs) const;

            /**
             * Compares the evaluation with another.
             *
             * @param rhs Right-hand side.
             * @return true if this > `rhs`.
             */
            bool operator > (const Evaluation& rhs) const;

            /**
             * Compares the evaluation with another.
             *
             * @param rhs Right-hand side.
             * @return true if this >= `rhs`.
             */
            bool operator >= (const Evaluation& rhs) const;

            /**
             * Compares the evaluation with another.
             *
             * @param rhs Right-hand side.
             * @return true if this <= `rhs`.
             */
            bool operator <= (const Evaluation& rhs) const;

            /**
             * Compares the evaluation with another.
             *
             * @param rhs Right-hand side.
             * @return true if this == `rhs`.
             */
            bool operator == (const Evaluation& rhs) const;
        protected:
            float eval;
            bool has_mate;
            int mate_in;
    };
}
