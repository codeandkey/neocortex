#pragma once

/**
 * Neural network evaluation
 */

/**
 * Input neurons:
 *
 * 64 * 12 neurons for each piece on each square.
 * 64 * 2 neurons for white king on each square
 * 64 * 2 neurons for black king on each square
 */

#include <string>

namespace neocortex {
    namespace nn {
        void load(std::string path);
        void save(std::string path);

        float evaluate(float* inp);
    }
}
