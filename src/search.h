#pragma once

#include "move.h"
#include "position.h"
#include "net.h"

#include <atomic>
#include <thread>
#include <mutex>

namespace neocortex {
        class Search {
        public:
            const float EXPLORATION = 1.414; // sqrt(2)
            const float POLICY_WEIGHT = 5.0f;

            /**
             * Builds a new search.
             *
             * @param net Search network.
             * @param bsize Batch size
             * @param posroot Search root
             */
            Search(nn::Network& net, int max_batch_size, Position& posroot);

            /**
             * Loads a new root position. Discards any cached search trees.
             *
             * @param posroot Search root
             */
            void reset(Position& posroot);

            /**
             * Searches the current root.
             *
             * @param search_time Maximum time to spend on search (milliseconds)
             * @param search_iterations Maximum iterations to search, or -1 to only use time limit
             * @param mcts_counts MCTS node count outputs (normalized)
             *
             * @return Selected action
             */
            int search(int search_time, std::vector<float>* mcts_counts);

            /**
             * Advances the search root position and tree.
             *
             * @param action Move to make.
             */
            void do_action(int action);

            /**
             * Gets a reference to the root position.
             *
             * @return Reference to root position.
             */
            Position& get_root();

            /**
             * Gets a reference to the search network.
             *
             * @return Reference to network.
             */
            nn::Network& get_network();

        private:
            struct Node {
                Node(Node* parent = NULL, int action = move::null()) {
                    n = 0;
                    terminal = -3;
                    q = w = 0.0f;
                    p = -1.0f;
                    this->action = action;
                    this->parent = parent;
                    this->lock = std::make_unique<std::mutex>();
                }

                int n, action, terminal; // -1: loss, 0: draw, (-2): not terminal, (-3): unknown
                float q, w;
                double p;
                Node* parent;
                std::unique_ptr<std::mutex> lock;
                std::vector<Node> children;

                void backprop(float v) {
                    ++n;
                    w += v;
                    q = w / (float) n;
                    if (parent) parent->backprop(-v);
                }
            };

            class Worker {
                /**
                 * Initializes a new search worker.
                 *
                 * @param bsize Batch size
                 */
                Worker(int bsize);

                /**
                 * Starts searching.
                 *
                 * @param root Tree root
                 */
                void start(nn::Network& net, Node& root);

                /**
                 * Signals the worker to stop searching.
                 */
                void stop();

                /**
                 * Joins the search thread.
                 * Blocks until the search thread is joined.
                 */
                void join();

                /**
                 * Advances the internal position.
                 *
                 * @param action Action to take.
                 */
                void advance(int action);

                /**
                 * Gets the status of the worker thread.
                 *
                 * @return status string
                 */
                std::string get_state();

            private:
                void set_state(std::string val);
                void worker(nn::Network& net, Node& root);

                int build_batch(Node& root, int allocated, int depth = 0, int offset = 0);

                std::atomic<bool> running;
                std::atomic<int> batch_avg, exec_avg, max_depth, terminal_count, pos_count;

                std::string state;
                std::mutex state_lock;
                std::thread worker_thread;

                int bsize;

                std::vector<Node*> batch_nodes;
                std::vector<float> board_input_layer;
                std::vector<float> lmm_input_layer;

                Position position;
            };

            Node root;
            int num_threads, max_batchsize_per_thread;
            nn::Network net;
            std::vector<Worker> workers;
        };
}
