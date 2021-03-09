#pragma once

#include "input.h"
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

			Search(std::string net_path, int max_batch_size, int num_threads, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

			void reset(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

			int search(int search_time, std::vector<float>* mcts_counts);

			void do_action(int action);

			int color_to_move();

			std::string get_name();

			std::vector<float> get_lmm_frame();
			std::vector<float> get_input_frame();

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

			Node root;
			int num_threads, max_batchsize_per_thread, max_depth;
			std::vector<nn::Network> nets;

			std::atomic<bool> running;
			std::atomic<int> pos_count, terminal_count, batch_avg, exec_avg;

			std::vector<Position> positions;
			std::vector<Input> inputs[2];
			std::vector<std::vector<Node*>> batch_nodes;

			std::string thread_states;
			std::mutex thread_states_lock;

			void worker(int id);
			int build_batch(Node& node, int threadid, int allocated, int depth=0, int offset=0);
		};
}
