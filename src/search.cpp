#include "search.h"
#include "log.h"
#include "color.h"

#include <chrono>
#include <initializer_list>
#include <random>
#include <iterator>
#include <cmath>

using namespace neocortex;

Search::Search(nn::Network& net, int max_batchsize_per_thread, int num_threads, Position& posroot) : net(net) {
	this->max_batchsize_per_thread = max_batchsize_per_thread;
	this->num_threads = num_threads;

    for (int i = 0; i < num_threads; ++i) {
        workers.push_back(Worker(max_batchsize_per_thread));
    }

	reset(posroot);
}

void Search::reset(Position& posroot) {
	positions.clear();

	for (int i = 0; i < num_threads; ++i) {
		positions.push_back(posroot);
	}

	root = Node();
}


int Search::search(int search_time, std::vector<float>* mcts_counts) {
	neocortex_info("%s: searching on %s for %d ms, %d batch, %d threads\n",
		nets[0].get_name().c_str(),
		positions[0].to_fen().c_str(),
		search_time,
		max_batchsize_per_thread,
		num_threads
	);

	// Set timepoint
	auto starttime = util::time_now();

    // Start workers
    for (auto& i : workers) {
        i.start(net, root);
    }

	// Wait for time to expire
	const int delay = 250;
	int elapsed = 0;

	while (elapsed <= search_time) {
		size_t progress = (elapsed * 10) / search_time;

		std::string progbar(progress + 1, '#');
		std::string emptybar(9 - progress, '-');

		int best_n = -1;
		float value = 0.0f;

		for (auto& i : root.children) {
			if (i.n >= best_n) {
				best_n = i.n;
				value = (i.n == best_n) ? (value + i.q) / 2.0f : i.q;
			}
		}

        int total_pos_count = 0, total_terminal_count = 0;
        int max_depth = -1;

        // Print info from each worker
        for (int i = 0; i < num_threads; ++i) {
            std::string state = workers[i].get_state();
            fprintf(stderr, "%s\n", state.c_str());

            total_pos_count += workers[i].pos_count;
            total_terminal_count += workers[i].terminal_count;

            if (workers[i].max_depth > max_depth) {
                max_depth = workers[i].max_depth;
            }
        }

		neocortex_info("Iterating [%s%s] %c%+3.2f%c %5d evaluated | %3.2fs elapsed | %6.2f EPS | %3d max depth          \r",
			progbar.c_str(),
			emptybar.c_str(),
			(best_n < 0) ? '(' : ' ',
			value,
			(best_n < 0) ? ')' : ' ',
            total_pos_count,
			(float)elapsed / 1000.0f,
			total_pos_count / ((elapsed + 1) / 1000.0f),
			max_depth
		);

		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(delay));

		if (total_pos_count == 0 && total_terminal_count == 0) {
			starttime = util::time_now();
		}

		elapsed = util::time_elapsed_ms(starttime);
	}

	fprintf(stderr, "\n");
	neocortex_info("Waiting for threads..\n");

	// Set stop flag, rejoin threads
	for (auto& t : workers) {
		t.stop();
	}

	for (auto& t : workers) {
		t.join();
	}

	std::vector<int> n_dist;

	if (mcts_counts) mcts_counts->resize(4096, 0.0f);

	for (int i = 0; i < root.children.size(); ++i) {
		n_dist.push_back(root.children[i].n);
	}

	// Perform ND child selection
	std::random_device device;
	std::mt19937 rng(device());
	std::discrete_distribution<> dist(n_dist.begin(), n_dist.end());

	int chosen = root.children[dist(rng)].action;

	// Print debug info
	neocortex_info("Picking from %d children:\n", root.children.size());

	std::vector<std::pair<int, int>> n_pairs;

	int n_total = 0;

	for (int i = 0; i < root.children.size(); ++i) {
		n_total += root.children[i].n;
		n_pairs.push_back({ root.children[i].n, i });
	}

	std::sort(n_pairs.begin(), n_pairs.end(), [&](auto& a, auto& b) { return a.first > b.first; });

	for (int i = 0; i < n_pairs.size(); ++i) {
		auto& child = root.children[n_pairs[i].second];
		std::string movestr = move::to_uci(child.action);

		for (int i = 0; i < root.children.size(); ++i) {
			if (positions[0].get_color_to_move() == color::WHITE) {
				if (mcts_counts) (*mcts_counts)[move::src(child.action) * 64 + move::dst(child.action)] = child.n / (float)n_total;
			}
			else {
				if (mcts_counts) (*mcts_counts)[(63 - move::src(child.action)) * 64 + (63 - move::dst(child.action))] = child.n / (float)n_total;
			}
		}

		neocortex_info(
			"%s> %5s  %03.1f%% | N=%4d | Q=%+04.2f | W=%+04.2f | P=%05.3f%%\n",
			(child.action == chosen) ? "##" : "  ",
			movestr.c_str(),
			100.0f * (float)child.n / (float)n_total,
			child.n,
			child.q,
			child.w,
			100.0f * (float)child.p
		);
	}

	std::string movestr = move::to_uci(chosen);
	neocortex_info("Selecting move %s\n", movestr.c_str());

	return chosen;
}

void Search::do_action(int action) {
	// Advance all boards
	for (int i = 0; i < num_threads; ++i) {
		positions[i].make_move(action);
	}

	// Preserve child if exists, otherwise generate a new root
	if (root.children.empty()) {
		root = Node();
		root.action = action;
	}
	else {
		// Locate child with matching action
		for (int i = 0; i < root.children.size(); ++i) {
			if (root.children[i].action == action) {
				Node tmp = std::move(root.children[i]);
				root = std::move(tmp);
				root.parent = NULL;

				// Root children parent pointer need to be updated
				for (auto& c : root.children) {
					c.parent = &root;
				}

				return;
			}
		}

		throw std::runtime_error("Couldn't locate action in children!");
	}
}

nn::Network& Search::get_network() {
    return net;
}

Search::Worker::Worker(int bsize) {
    running = false;
    state = "not started";
    board_input_layer.resize(bsize * 8 * 8 * 85);
    lmm_input_layer.resize(bsize * 4096);
    this->bsize = bsize;
}

void Search::Worker::start(nn::Network& net, Node& root) {
    worker_thread = std::thread(Search::Worker::worker, net, root);
}

void Search::Worker::join() {
    worker_thread.join();
}

void Search::Worker::stop() {
    running = false;
}

void Search::Worker::worker(nn::Network& net, Node& root) {
	while (running) {
		auto batch_starttime = util::time_now();

        set_state("building");

        batch_nodes.clear();

		int batch_size = build_batch(root, bsize);

		batch_avg = (util::time_elapsed_ms(batch_starttime) + batch_avg) / 2;

		if (batch_size > 0) {
			auto exec_starttime = util::time_now();

            set_state("evaluate");

			auto results = net.evaluate(
                board_input_layer,
                lmm_input_layer,
				bsize
			);

			auto policy = results[0].get_data<float>();
			auto value = results[1].get_data<float>();

            set_state("backprop");

			bool mirror_policy = (position.get_color_to_move() == color::BLACK);

			// Assign policy and backprop values
			for (size_t i = 0; i < batch_size; ++i) {
				batch_nodes[id][i]->backprop(value[i]);

				assert(!std::isnan(value[i]));

				double p_total = 0.0f;
				if (mirror_policy) {
					// Black POV, the policy needs to be mirrored.
					for (auto& c : batch_nodes[id][i]->children) {
						c.p = policy[4096 * i + (63 - move::src(c.action)) * 64 + (63 - move::dst(c.action))];
						p_total += c.p;
						assert(!std::isnan(c.p));
					}
				}
				else {
					for (auto& c : batch_nodes[id][i]->children) {
						c.p = policy[4096 * i + move::src(c.action) * 64 + move::dst(c.action)];
						p_total += c.p;
						assert(!std::isnan(c.p));
					}
				}

				for (auto& c : batch_nodes[id][i]->children) {
					c.p /= p_total;
				}

				batch_nodes[id][i]->lock->unlock();
			}

			// Update total evaluations
			pos_count += batch_size;

			// Update average exec/backprop time
			exec_avg = (util::time_elapsed_ms(exec_starttime) + exec_avg) / 2;
		}
	}
}

int Search::Worker::build_batch(Node& node, int allocated, int depth, int offset) {
	if (depth > max_depth) {
		max_depth = depth;
	}

	// Test if node has children
	if (node.children.empty()) {
		// Check if terminal

		if (node.terminal == -3) {
			// Check result of game
			if (!position.is_game_over(&node.terminal)) {
				node.terminal = -2;
			}
		}

		if (node.terminal > -2) {
			// Backprop the terminal value and return.
			// The game is either a loss (checkmate, color to move has lost) or a draw

			if (node.terminal == 0) {
				node.backprop(0.0f);
			}
			else {
				node.backprop(-1.0f);
			}

			++terminal_count;
			return 0;
		}
		else {
			// Try to claim node
			if (node.lock->try_lock()) {
				// Possible weird codepath
				if (node.children.size() > 0) {
					node.lock->unlock();
					return 0;
				}

                // Generate moves and LMM
                int legal_moves[MAX_PL_MOVES];
                int num_moves = position.get_lmm(&lmm_input_layer[offset * 4096], legal_moves);

				for (int i = 0; i < num_moves; ++i) {
                    node.children.emplace_back(&node, moves[i]);
				}

				batch_nodes[offset] = &node;
				return 1;
			}
			else {
				// Node already claimed, skip
				return 0;
			}
		}
	}
	else {
		if (!node.lock->try_lock()) {
			return 0;
		}

		// Node has children, order them by UCT
		std::vector<std::pair<float, int>> uct_pairs;
		float uct_total = 0.0f;

		for (int i = 0; i < node.children.size(); ++i) {
			auto& child = node.children[i];
			float uct = child.q + child.p * POLICY_WEIGHT + EXPLORATION * std::sqrt(((float) std::log(node.n + 1) / ((float) child.n + 1.0f)));
			uct_total += uct;
			uct_pairs.push_back({ uct, i });

			assert(!std::isnan(uct));
		}

		node.lock->unlock();

		std::sort(uct_pairs.begin(), uct_pairs.end(), [&](auto& a, auto& b) { return a.first > b.first; });

		// Start allocating batches
		int total_batches = 0;

		for (int i = 0; i < uct_pairs.size(); ++i) {
			if (allocated <= 0) {
				break;
			}

			Node& child = node.children[uct_pairs[i].second];
			int child_alloc = std::ceil(uct_pairs[i].first * float(allocated) / uct_total);

			uct_total -= uct_pairs[i].first;

			if (child_alloc <= 0) {
				break;
			}

			if (child_alloc > allocated) {
				child_alloc = allocated;
			}

			position.make_move(child.action);

			int new_batches = build_batch(child, child_alloc, depth + 1, offset, batch_nodes);

			position.unmake_move();

			allocated -= new_batches;
			total_batches += new_batches;
			offset += new_batches;

			if (allocated == 0) {
				return total_batches;
			}
		}

		return total_batches;
	}
}
