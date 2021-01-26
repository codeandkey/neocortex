#include "nn.h"
#include "log.h"
#include "piece.h"
#include "util.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cerrno>

#include <mutex>
#include <random>
#include <shared_mutex>

using namespace neocortex;

static int nn::_nn_num_layers;
static int* nn::_nn_layer_sizes;
static float** _nn_connection_weights; // [layer][{src index} * {layer size} + {dst index}] (contains 1 extra "layer" for output weights)
static float** _nn_node_biases; // [layer][node]
static bool _nn_loaded;
static std::shared_mutex _nn_mutex;

static void _nn_compute_layer(float* inp, int inp_len, float* dst, int dst_len, float* weights, float* biases);

void nn::generate() {
    std::random_device rng;
    std::mt19937_64 twister(rng());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    _nn_mutex.lock();

    _nn_num_layers = NUM_DEFAULT_LAYERS;
    _nn_layer_sizes = new int[_nn_num_layers];

    // Copy default layer size
    for (int i = 0; i < _nn_num_layers; ++i) {
        _nn_layer_sizes[i] = DEFAULT_LAYERS[i];
    }

    // Initialize connection weights
    _nn_connection_weights = new float*[(size_t) _nn_num_layers + 1];

    _nn_connection_weights[0] = new float[(size_t) INPUT_NODES * _nn_layer_sizes[0]];
    _nn_connection_weights[_nn_num_layers] = new float[_nn_layer_sizes[_nn_num_layers - 1]];

    for (int i = 1; i < _nn_num_layers; ++i) {
        _nn_connection_weights[i] = new float[(size_t) _nn_layer_sizes[i] * _nn_layer_sizes[i - 1]];
    }

    // Generate first connection weights
    for (int i = 0; i < INPUT_NODES * _nn_layer_sizes[0]; ++i) {
        _nn_connection_weights[0][i] = dist(twister);
    }

    // Generate last connection weights
    for (int i = 0; i < _nn_layer_sizes[_nn_num_layers - 1]; ++i) {
        _nn_connection_weights[_nn_num_layers][i] = dist(twister);
    }

    // Generate inner layer connection weights
    for (int i = 1; i < _nn_num_layers; ++i) {
        for (int j = 0; j < _nn_layer_sizes[i] * _nn_layer_sizes[i - 1]; ++j) {
            _nn_connection_weights[i][j] = dist(twister);
        }
    }

    // Init node bias
    
    _nn_node_biases = new float*[_nn_num_layers];

    // Generate node bias
        
    for (int i = 0; i < _nn_num_layers; ++i) {
        _nn_node_biases[i] = new float[_nn_layer_sizes[i]];

        for (int j = 0; j < _nn_layer_sizes[i]; ++j) {
            _nn_node_biases[i][j] = 0.0f;
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
    _nn_layer_sizes = new int[_nn_num_layers];

    if (fread(_nn_layer_sizes, sizeof _nn_layer_sizes[0], _nn_num_layers, inp) != _nn_num_layers) {
        goto nn_load_read_fail;
    }

    // Verify input size
    int input_size;

    if (fread(&input_size, sizeof(input_size), 1, inp) != 1) {
        goto nn_load_read_fail;
    }

    if (input_size != INPUT_NODES) {
        neocortex_error("Input layer size mismatch in %s: expected %d, read %d", path.c_str(), INPUT_NODES, input_size);
        goto nn_load_read_fail;
    }

    // Initialize connection weights
    
    _nn_connection_weights = new float*[_nn_num_layers + 1];

    _nn_connection_weights[0] = new float[INPUT_NODES * _nn_layer_sizes[0]];
    _nn_connection_weights[_nn_num_layers] = new float[_nn_layer_sizes[_nn_num_layers - 1]];

    for (int i = 1; i < _nn_num_layers; ++i) {
        _nn_connection_weights[i] = new float[_nn_layer_sizes[i] * _nn_layer_sizes[i - 1]];
    }

    // Read first connection weights
    if (fread(_nn_connection_weights[0], sizeof(**_nn_connection_weights), INPUT_NODES * _nn_layer_sizes[0], inp) != INPUT_NODES * _nn_layer_sizes[0]) {
        goto nn_load_read_fail;
    }

    // Read inner layer connection weights
    for (int i = 1; i < _nn_num_layers; ++i) {
        if (fread(_nn_connection_weights[i], sizeof(**_nn_connection_weights), _nn_layer_sizes[i] * _nn_layer_sizes[i - 1], inp) != _nn_layer_sizes[i] * _nn_layer_sizes[i - 1]) {
            goto nn_load_read_fail;
        }
    }

    // Read last connection weights
    if (fread(_nn_connection_weights[_nn_num_layers], sizeof(**_nn_connection_weights), _nn_layer_sizes[_nn_num_layers - 1], inp) != _nn_layer_sizes[_nn_num_layers - 1]) {
        goto nn_load_read_fail;
    }

    // Init node bias
    
    _nn_node_biases = new float*[_nn_num_layers];

    // Parse node bias
        
    for (int i = 0; i < _nn_num_layers; ++i) {
        _nn_node_biases[i] = new float[_nn_layer_sizes[i]];

        if (fread(_nn_node_biases[i], sizeof **_nn_node_biases, _nn_layer_sizes[i], inp) != _nn_layer_sizes[i]) {
            goto nn_load_read_fail;
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
    
    // Write first connection weights
    if (fwrite(_nn_connection_weights[0], sizeof(**_nn_connection_weights), INPUT_NODES * _nn_layer_sizes[0], out) != INPUT_NODES * _nn_layer_sizes[0]) {
        goto nn_save_write_fail;
    }

    // Read inner layer connection weights
    for (int i = 1; i < _nn_num_layers; ++i) {
        if (fwrite(_nn_connection_weights[i], sizeof(**_nn_connection_weights), _nn_layer_sizes[i] * _nn_layer_sizes[i - 1], out) != _nn_layer_sizes[i] * _nn_layer_sizes[i - 1]) {
            goto nn_save_write_fail;
        }
    }

    // Write last connection weights
    if (fwrite(_nn_connection_weights[_nn_num_layers], sizeof(**_nn_connection_weights), _nn_layer_sizes[_nn_num_layers - 1], out) != _nn_layer_sizes[_nn_num_layers - 1]) {
        goto nn_save_write_fail;
    }

    // Write node bias
    
    for (int i = 0; i < _nn_num_layers; ++i) {
        if (fwrite(_nn_node_biases[i], sizeof **_nn_node_biases, _nn_layer_sizes[i], out) != _nn_layer_sizes[i]) {
            goto nn_save_write_fail;
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
        for (int i = 0; i < _nn_num_layers; ++i) {
            delete[] _nn_connection_weights[i];
        }

        delete[] _nn_connection_weights;
        _nn_connection_weights = NULL;
    }

    if (_nn_node_biases) {
        for (int i = 0; i < _nn_num_layers; ++i) {
            delete[] _nn_node_biases[i];
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

void _nn_compute_layer(float* inp, int inp_len, float* dst, int dst_len, float* weights, float* biases) {
    for (int i = 0; i < dst_len; ++i) {
        dst[i] = biases[i];

        for (int j = 0; j < inp_len; ++j) {
            dst[i] += weights[j * dst_len + i] * inp[j];
        }

        if (dst[i] < 0.0f) {
            dst[i] = 0.0f; // Rectified linear activation
        }
    }
}

float nn::evaluate(ComputeState& s, Board& b, int ctm) {
    _nn_mutex.lock_shared();

    // Compute first layer explicitly, as two inputs are required

    float* own_input = b.get_nn_input(ctm);
    float* opp_input = b.get_nn_input(!ctm);

    for (int i = 0; i < _nn_layer_sizes[0]; ++i) {
        s.layers[0][i] = _nn_node_biases[0][i];

        // Apply first input half
        for (int j = 0; j < 20480; ++j) {
            s.layers[0][i] += _nn_connection_weights[0][j * _nn_layer_sizes[i] + i] * own_input[j];
        }

        // Apply second input half
        for (int j = 0; j < 20480; ++j) {
            s.layers[0][i] += _nn_connection_weights[0][(j + 20480) * _nn_layer_sizes[i] + i] * opp_input[j];
        }

        // ReLU activation
        if (s.layers[0][i] < 0.0f) {
            s.layers[0][i] = 0.0f;
        }
    }

    // Compute hidden layers

    for (int i = 1; i < _nn_num_layers; ++i) {
        for (int dst = 0; dst < _nn_layer_sizes[i]; ++dst) {
            s.layers[i][dst] = _nn_node_biases[i][dst];

            for (int src = 0; src < _nn_layer_sizes[i - 1]; ++src) {
                s.layers[i][dst] += s.layers[i - 1][src] * _nn_connection_weights[i][src * _nn_layer_sizes[i] + dst];
            }

            if (s.layers[i][dst] < 0.0f) {
                s.layers[i][dst] = 0.0f;
            }
        }
    }

    // Compute output node

    float output = 0.0f; // TODO: consider output bias

    for (int i = 0; i < _nn_layer_sizes[_nn_num_layers - 1]; ++i) {
        output += s.layers[_nn_num_layers - 1][i] * _nn_connection_weights[_nn_num_layers][i];
    }

    // No activation func. on output, just get the raw value.

    _nn_mutex.unlock_shared();
    return output;
}
