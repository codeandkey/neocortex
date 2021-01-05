#pragma once

/**
 * Neural network evaluation
 */

/**
 * Input layer:
 *
 * 64 * 12 neurons for each piece on each square.
 * Extra 64 * 2 neurons for white king on each square
 * Extra 64 * 2 neurons for black king on each square
 *
 * Total: 1024 nodes
 *
 * Hidden layers (2 of each, for each color)
 *
 * 1024 nodes * 4 primary hidden layers
 *
 * 512 nodes
 *
 * 256 nodes
 *
 * 128 nodes
 *
 * Total: 7 hidden layers, 4992 nodes
 *
 * Output layer:
 *
 * 1 node: 0-signal is worst and 1-signal is best.
 *
 */

#include <string>

namespace neocortex {
    namespace nn {
        constexpr int DEFAULT_LAYERS[] = {
            1024,
            1024,
            1024,
            1024,
            512,
            256,
            128
        };

        constexpr int NUM_DEFAULT_LAYERS = sizeof(DEFAULT_LAYERS) / sizeof(DEFAULT_LAYERS[0]);
        constexpr int INPUT_NODES = 1024;

        void generate();
        void load(std::string path);
        void save(std::string path);
        void cleanup();

        float evaluate(bool* inp);
    }
}
