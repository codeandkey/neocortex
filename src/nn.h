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

#include "board.h"

#include <cstdint>
#include <string>

namespace neocortex {
    namespace nn {
        constexpr int DEFAULT_LAYERS[] = {
            8,
            8,
            8,
        };

        constexpr int NUM_DEFAULT_LAYERS = sizeof(DEFAULT_LAYERS) / sizeof(DEFAULT_LAYERS[0]);
        constexpr int INPUT_NODES = 40960;

        constexpr const char* DEFAULT_PATH = "nc.nn";

        extern unsigned int _nn_num_layers;
        extern unsigned int* _nn_layer_sizes;

        void generate();
        void load(std::string path);
        void save(std::string path);
        void cleanup();

        struct ComputeState {
            ComputeState() {
                layers = new float* [_nn_num_layers];

                for (unsigned i = 0; i < _nn_num_layers; ++i) {
                    layers[i] = new float[_nn_layer_sizes[i]];
                }
            }

            ~ComputeState() {
                for (unsigned i = 0; i < _nn_num_layers; ++i) {
                    delete[] layers[i];
                }

                delete[] layers;
                layers = NULL;
            }

            float** layers;
        };

        float evaluate(ComputeState& s, Board& b, int ctm);
    }
}
