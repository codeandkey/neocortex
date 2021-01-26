#pragma once

/**
 * Neural network evaluation
 */

/**
 * Input layer:
 *
 * {Own king location} X {Own non-king locations}: 64 * (5 * 64) = 20480 nodes
 * {Opponent king location} X {Opponent non-king locations} : 20480 nodes
 *
 * total: 40960 input nodes
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
            2048,
            1024,
            1024,
            512,
            256,
            128
        };

        constexpr int NUM_DEFAULT_LAYERS = sizeof(DEFAULT_LAYERS) / sizeof(DEFAULT_LAYERS[0]);
        constexpr int INPUT_NODES = 40960;

        constexpr const char* DEFAULT_PATH = "nc.nn";

        void generate();
        void load(std::string path);
        void save(std::string path);
        void cleanup();

        float evaluate(float* inp);
    }
}
