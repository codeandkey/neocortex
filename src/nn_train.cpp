#include "nn_train.h"
#include "nn.h"
#include "position.h"
#include "search.h"
#include "log.h"

#include <atomic>

using namespace neocortex;

static std::thread _nn_training_thread;
static std::atomic<bool> _nn_train_running, _nn_train_abort;

static void _nn_train_classical(int ms);

void nn::start_classical_training(int ms) {
    nn::stop_training();
    _nn_training_thread = std::thread(_nn_train_classical, ms);
    _nn_train_running = true;
}

void nn::stop_training() {
    if (_nn_train_running) {
        _nn_train_abort = true;
        _nn_training_thread.join();
        _nn_train_running = false;
        _nn_train_abort = false;
    }
}

void _nn_train_classical(int ms) {
    // Unsupervised network learning
    // NN is trained to approximate classical nc search for n milliseconds
    
    neocortex_debug("Starting classical training, movetime=%dms\n", ms);

    nn::ComputeState cs;
    
    while (!_nn_train_abort) {
        search::Search s;
        Position p;

        // Play a game and have the NN decide the moves by simple 1-depth search.
        float nn_eval = nn::evaluate(cs, p.get_board(), p.get_color_to_move());

        Move best_move;
        int score;

        // Get the sync search value.
        s.go([&](search::SearchInfo inf) {
            score = inf.score;
        }, [&](Move m) {
            best_move = m;
        }, -1, -1, -1, -1, -1, ms, false);

        s.wait();

        // Compute error
        float err_raw = (score * 100.0f) - nn_eval;

        // Perform backpropagation on network to train.

    }
}
