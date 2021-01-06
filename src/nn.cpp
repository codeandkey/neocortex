#include "nn.h"
#include "log.h"
#include "util.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cerrno>

#include <mutex>
#include <random>
#include <shared_mutex>

using namespace neocortex;

static uint16_t _nn_num_layers;
static uint16_t* _nn_layer_sizes;
static float*** _nn_connection_weights;
static float*** _nn_node_biases;
static bool _nn_loaded;
static std::shared_mutex _nn_mutex;

void nn::generate() {
    std::random_device rng;
    std::mt19937_64 twister(rng());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    _nn_mutex.lock();

    _nn_num_layers = NUM_DEFAULT_LAYERS;
    _nn_layer_sizes = new uint16_t[_nn_num_layers];

    // Copy default layer size
    for (int i = 0; i < _nn_num_layers; ++i) {
        _nn_layer_sizes[i] = DEFAULT_LAYERS[i];
    }

    // Initialize connection weights
    _nn_connection_weights = new float**[2];

    for (int c = 0; c < 2; ++c) {
        _nn_connection_weights[c] = new float*[_nn_num_layers + 2];

        _nn_connection_weights[c][0] = new float[INPUT_NODES * _nn_layer_sizes[0]];
        _nn_connection_weights[c][_nn_num_layers + 1] = new float[_nn_layer_sizes[_nn_num_layers - 1]];

        for (int i = 0; i < _nn_num_layers - 1; ++i) {
            _nn_connection_weights[c][i + 1] = new float[_nn_layer_sizes[i] * _nn_layer_sizes[i + 1]];
        }

        // Generate first connection weights
        for (int i = 0; i < INPUT_NODES * _nn_layer_sizes[0]; ++i) {
            _nn_connection_weights[c][0][i] = dist(twister);
        }

        // Generate last connection weights
        for (int i = 0; i < _nn_layer_sizes[_nn_num_layers - 1]; ++i) {
            _nn_connection_weights[c][_nn_num_layers + 1][i] = dist(twister);
        }

        // Generate inner layer connection weights
        for (int i = 0; i < _nn_num_layers - 1; ++i) {
            for (int j = 0; j < _nn_layer_sizes[i]; ++j) {
                _nn_connection_weights[c][i][j] = dist(twister);
            }
        }
    }

    // Init node bias
    
    _nn_node_biases = new float**[2];

    for (int c = 0; c < 2; ++c) {
        _nn_node_biases[c] = new float*[_nn_num_layers];

        for (int i = 0; i < _nn_num_layers; ++i) {
            _nn_node_biases[c][i] = new float[_nn_layer_sizes[i]];
        }

        // Generate node bias
        
        for (int i = 0; i < _nn_num_layers; ++i) {
            _nn_node_biases[c][i] = new float[_nn_layer_sizes[i]];

            for (int j = 0; j < _nn_layer_sizes[i]; ++j) {
                _nn_node_biases[c][i][j] = 0.0f;
            }
        }
    }

    _nn_loaded = true;

    neocortex_debug("Generated new NN\n");

    _nn_mutex.unlock();
}

void nn::load(std::string path) {
    _nn_mutex.lock();

    // Read NN configuration and parameters from file
    FILE* inp = fopen(path.c_str(), "rb");

    if (!inp) {
        _nn_mutex.unlock();
        throw util::fmterr("Failed to open %s for reading: %s", path.c_str(), strerror(errno));
    }

    // Read number of layers

    if (fread(&_nn_num_layers, sizeof(_nn_num_layers), 1, inp) != 1) {
        goto nn_load_read_fail;
    }

    // Read layer sizes
    _nn_layer_sizes = new uint16_t[_nn_num_layers];

    if (fread(_nn_layer_sizes, sizeof _nn_layer_sizes[0], _nn_num_layers, inp) != _nn_num_layers) {
        goto nn_load_read_fail;
    }

    // Verify input size
    uint16_t input_size;

    if (fread(&input_size, sizeof(input_size), 1, inp) != 1) {
        goto nn_load_read_fail;
    }

    if (input_size != INPUT_NODES) {
        neocortex_error("Input layer size mismatch in %s: expected %d, read %d", path.c_str(), INPUT_NODES, input_size);
        goto nn_load_read_fail;
    }

    // Initialize connection weights
    
    _nn_connection_weights = new float**[2];

    for (int c = 0; c < 2; ++c) {
        _nn_connection_weights[c] = new float*[_nn_num_layers + 2];

        _nn_connection_weights[c][0] = new float[INPUT_NODES * _nn_layer_sizes[0]];
        _nn_connection_weights[c][_nn_num_layers + 1] = new float[_nn_layer_sizes[_nn_num_layers - 1]];

        for (int i = 0; i < _nn_num_layers - 1; ++i) {
            _nn_connection_weights[c][i + 1] = new float[_nn_layer_sizes[i] * _nn_layer_sizes[i + 1]];
        }

        // Read first connection weights
        if (fread(_nn_connection_weights[c][0], sizeof(***_nn_connection_weights), INPUT_NODES * _nn_layer_sizes[0], inp) != INPUT_NODES * _nn_layer_sizes[0]) {
            goto nn_load_read_fail;
        }

        // Read last connection weights
        if (fread(_nn_connection_weights[c][_nn_num_layers + 1], sizeof(***_nn_connection_weights), _nn_layer_sizes[_nn_num_layers - 1], inp) != _nn_layer_sizes[_nn_num_layers - 1]) {
            goto nn_load_read_fail;
        }

        // Read inner layer connection weights
        for (int i = 0; i < _nn_num_layers - 1; ++i) {
            if (fread(_nn_connection_weights[c][i + 1], sizeof(***_nn_connection_weights), _nn_layer_sizes[i] * _nn_layer_sizes[i + 1], inp) != _nn_layer_sizes[i] * _nn_layer_sizes[i + 1]) {
                goto nn_load_read_fail;
            }
        }
    }

    // Init node bias
    
    _nn_node_biases = new float**[2];

    for (int c = 0; c < 2; ++c) {
        _nn_node_biases[c] = new float*[_nn_num_layers];

        for (int i = 0; i < _nn_num_layers; ++i) {
            _nn_node_biases[c][i] = new float[_nn_layer_sizes[i]];
        }

        // Parse node bias
        
        for (int i = 0; i < _nn_num_layers; ++i) {
            _nn_node_biases[c][i] = new float[_nn_layer_sizes[i]];

            if (fread(_nn_node_biases[c][i], sizeof ***_nn_node_biases, _nn_layer_sizes[i], inp) != _nn_layer_sizes[i]) {
                goto nn_load_read_fail;
            }
        }
    }

    _nn_loaded = true;
    _nn_mutex.unlock();

    neocortex_debug("Loaded NN from %s\n", path.c_str());

    fclose(inp);
    return;

    // Generic handler for file read errors
    nn_load_read_fail:
    _nn_mutex.unlock();
    fclose(inp);
    nn::cleanup();
    throw util::fmterr("Failed to read weights from %s: invalid data", path.c_str());
}

void nn::save(std::string path) {
    _nn_mutex.lock();

    assert(_nn_loaded);

    // Read NN configuration and parameters from file
    FILE* out = fopen(path.c_str(), "wb");

    if (!out) {
        _nn_mutex.unlock();
        throw util::fmterr("Failed to open %s for writing: %s", path.c_str(), strerror(errno));
    }

    // Write number of layers

    if (fwrite(&_nn_num_layers, sizeof(_nn_num_layers), 1, out) != 1) {
        goto nn_save_write_fail;
    }

    // Write layer sizes
    if (fwrite(_nn_layer_sizes, sizeof _nn_layer_sizes[0], _nn_num_layers, out) != _nn_num_layers) {
        goto nn_save_write_fail;
    }

    // Verify input size
    uint16_t input_size;

    input_size = INPUT_NODES;

    if (fwrite(&input_size, sizeof(input_size), 1, out) != 1) {
        goto nn_save_write_fail;
    }

    // Write connection weights
    
    for (int c = 0; c < 2; ++c) {
        // Write first connection weights
        if (fwrite(_nn_connection_weights[c][0], sizeof(***_nn_connection_weights), INPUT_NODES * _nn_layer_sizes[0], out) != INPUT_NODES * _nn_layer_sizes[0]) {
            goto nn_save_write_fail;
        }

        // Read inner layer connection weights
        for (int i = 0; i < _nn_num_layers - 1; ++i) {
            if (fwrite(_nn_connection_weights[c][i + 1], sizeof(***_nn_connection_weights), _nn_layer_sizes[i] * _nn_layer_sizes[i + 1], out) != _nn_layer_sizes[i] * _nn_layer_sizes[i + 1]) {
                goto nn_save_write_fail;
            }
        }

        // Write last connection weights
        if (fwrite(_nn_connection_weights[c][_nn_num_layers + 1], sizeof(***_nn_connection_weights), _nn_layer_sizes[_nn_num_layers - 1], out) != _nn_layer_sizes[_nn_num_layers - 1]) {
            goto nn_save_write_fail;
        }
    }

    // Write node bias
    
    for (int c = 0; c < 2; ++c) {
        for (int i = 0; i < _nn_num_layers; ++i) {
            if (fwrite(_nn_node_biases[c][i], sizeof ***_nn_node_biases, _nn_layer_sizes[i], out) != _nn_layer_sizes[i]) {
                goto nn_save_write_fail;
            }
        }
    }

    _nn_mutex.unlock();

    neocortex_debug("Wrote NN to %s\n", path.c_str());

    fclose(out);
    return;

    // Generic handler for file read errors
    nn_save_write_fail:
    _nn_mutex.unlock();
    fclose(out);
    throw util::fmterr("Failed to read weights from %s: invalid data", path.c_str());
}

void nn::cleanup() {
    _nn_mutex.lock();

    if (_nn_connection_weights) {
        for (int c = 0; c < 2; ++c) {
            for (int i = 0; i < _nn_num_layers; ++i) {
                delete[] _nn_connection_weights[c][i];
            }

            delete[] _nn_connection_weights[c];
        }

        delete[] _nn_connection_weights;
        _nn_connection_weights = NULL;
    }

    if (_nn_node_biases) {
        for (int c = 0; c < 2; ++c) {
            for (int i = 0; i < _nn_num_layers; ++i) {
                delete[] _nn_node_biases[c][i];
            }

            delete[] _nn_node_biases[c];
        }

        delete[] _nn_node_biases;

        _nn_node_biases = NULL;
    }

    if (_nn_layer_sizes) {
        delete[] _nn_layer_sizes;
        _nn_layer_sizes = NULL;
    }

    _nn_num_layers = 0;
    _nn_loaded = false;

    _nn_mutex.unlock();
}
