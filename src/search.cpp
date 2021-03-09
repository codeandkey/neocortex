#include "search.h"
#include "log.h"
#include "color.h"

#include <chrono>
#include <initializer_list>
#include <random>
#include <iterator>
#include <cmath>

using namespace neocortex;

Search::Search(std::string path, int max_batchsize_per_thread, int num_threads, std::string fen) {
	//if (num_threads > 4) num_threads = 1;

	this->max_batchsize_per_thread = max_batchsize_per_thread;
	this->num_threads = num_threads;
	
	batch_nodes.resize(num_threads);

	for (auto& b : batch_nodes) {
		b.resize(max_batchsize_per_thread, NULL);
	}

	neocortex_info("Initializing %d networks..\n", num_threads);

	for (int i = 0; i < num_threads; ++i) {
		neocortex_info("Initializing network %d of %d..\n", i + 1, num_threads);
		nets.push_back(nn::Network(path));
	}

	thread_states = std::string(num_threads, ' ');

	reset(fen);
}

void Search::reset(std::string fen) {
	positions.clear();
	inputs[color::WHITE].clear();
	inputs[color::BLACK].clear();

	for (int i = 0; i < num_threads; ++i) {
		positions.push_back(Position(fen));
		inputs[color::WHITE].push_back(Input(max_batchsize_per_thread));
		inputs[color::BLACK].push_back(Input(max_batchsize_per_thread));
	}

	root = Node();

	// Initialize inputs
	for (int c = 0; c < 2; ++c) {
		for (auto& inp : inputs[c]) {
			for (int j = 0; j < max_batchsize_per_thread; ++j) {
				inp.write_frame(j, positions[0].get_board(), c, positions[0].num_repetitions(), positions[0].move_number(), positions[0].halfmove_clock());
			}	
		}
	}
}

int Search::build_batch(Node& node, int threadid, int allocated, int depth, int offset) {
	if (depth > max_depth) {
		max_depth = depth;
	}

	// Test if node has children
	if (node.children.empty()) {
		// Check if terminal

		if (node.terminal == -3) {
			// Check result of game
			if (!positions[threadid].is_game_over(&node.terminal)) {
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

				// Not terminal node, expand children
				int moves[MAX_PL_MOVES];
				int num_moves = positions[threadid].pseudolegal_moves(moves);
				int ctm = positions[threadid].get_color_to_move();

				inputs[ctm][threadid].clear_lmm(offset);

				for (int i = 0; i < num_moves; ++i) {
					if (positions[threadid].make_move(moves[i])) {
						inputs[ctm][threadid].write_lmm(offset, moves[i], ctm);
						node.children.emplace_back(&node, moves[i]);
					}

					positions[threadid].unmake_move();
				}

				batch_nodes[threadid][offset] = &node;
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

			/* Update inputs and make move for all allocated subbatches */
			for (int j = offset; j < offset + child_alloc; ++j) {
				for (int c = 0; c < 2; ++c) {
					inputs[c][threadid].push_frame(j);

					inputs[c][threadid].write_frame(
						j,
						positions[threadid].get_board(),
						c,
						positions[threadid].num_repetitions(),
						positions[threadid].move_number(),
						positions[threadid].halfmove_clock()
					);
				}
			}

			positions[threadid].make_move(child.action);

			int new_batches = build_batch(child, threadid, child_alloc, depth + 1, offset);

			positions[threadid].unmake_move();

			for (int j = offset; j < offset + child_alloc; ++j) {
				inputs[color::WHITE][threadid].pop_frame(j);
				inputs[color::BLACK][threadid].pop_frame(j);
			}

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

void Search::worker(int id) {
	while (running) {
		auto batch_starttime = util::time_now();

		thread_states_lock.lock();
		thread_states[id] = 'B';
		thread_states_lock.unlock();

		// Clear batch nodes
		batch_nodes[id] = std::vector<Node*>(max_batchsize_per_thread, NULL);

		int batch_size = build_batch(root, id, max_batchsize_per_thread);

		batch_avg = (util::time_elapsed_ms(batch_starttime) + batch_avg) / 2;

		if (batch_size > 0) {
			auto exec_starttime = util::time_now();

			thread_states_lock.lock();
			thread_states[id] = 'X';
			thread_states_lock.unlock();

			auto results = nets[id].evaluate(
				inputs[positions[id].get_color_to_move()][id].get_board_input(),
				inputs[positions[id].get_color_to_move()][id].get_lmm_input(),
				batch_size
			);

			auto policy = results[0].get_data<float>();
			auto value = results[1].get_data<float>();

			thread_states_lock.lock();
			thread_states[id] = '-';
			thread_states_lock.unlock();

			bool mirror_policy = (positions[id].get_color_to_move() == color::BLACK);

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

	// Start search threads
	std::vector<std::thread> threads;

	running = true;
	pos_count = 0;
	terminal_count = 0;
	max_depth = 0;

	for (int i = 0; i < num_threads; ++i) {
		threads.push_back(std::thread(&Search::worker, this, i));
	}

	// Wait for time to expire
	const int delay = 250;
	int elapsed = 0;

	while (elapsed <= search_time) {
		size_t progress = (elapsed * 10) / search_time;

		std::string progbar(progress + 1, '#');
		std::string emptybar(9 - progress, '-');

		thread_states_lock.lock();

		Node* best_child = NULL;
		int best_n = -1;
		float value = 0.0f;
		
		for (auto& i : root.children) {
			if (i.n >= best_n) {
				best_n = i.n;
				best_child = &i;
				value = (i.n == best_n) ? (value + i.q) / 2.0f : i.q;
			}
		}

		neocortex_info("Iterating [%s%s] [%s] %c%+3.2f%c %5d evaluated | %3.2fs elapsed | %6.2f EPS | %3d depth | %4dms batch | %4dms exec | %d terminals          \r",
			progbar.c_str(),
			emptybar.c_str(),
			thread_states.c_str(),
			(best_n < 0) ? '(' : ' ',
			value,
			(best_n < 0) ? ')' : ' ',
			pos_count.load(),
			(float)elapsed / 1000.0f,
			pos_count / ((elapsed + 1) / 1000.0f),
			max_depth,
			batch_avg.load(),
			exec_avg.load(),
			terminal_count.load()
		);

		thread_states_lock.unlock();

		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(delay));

		if (pos_count == 0 && terminal_count == 0) {
			starttime = util::time_now();
		}

		elapsed = util::time_elapsed_ms(starttime);
	}

	fprintf(stderr, "\n");
	neocortex_info("Waiting for threads..\n");

	// Set stop flag, rejoin threads
	running = false;

	for (auto& t : threads) {
		t.join();
	}

	std::vector<int> n_dist;

	if (mcts_counts) mcts_counts->resize(4096, 0.0f);

	for (int i = 0; i < root.children.size(); ++i) {
		n_dist.push_back(root.children[i].n);
		if (positions[0].get_color_to_move() == color::WHITE) {
			if (mcts_counts) (*mcts_counts)[move::src(root.children[i].action) * 64 + move::dst(root.children[i].action)] = root.children[i].n;
		}
		else {
			if (mcts_counts) (*mcts_counts)[(63 - move::src(root.children[i].action)) * 64 + (63 - move::dst(root.children[i].action))] = root.children[i].n;
		}
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
	// Advance all boards, inputs by action
	for (int i = 0; i < num_threads; ++i) {
		positions[i].make_move(action);

		for (int c = 0; c < 2; ++c) {
			for (int b = 0; b < max_batchsize_per_thread; ++b) {
				Input& input = inputs[c][i];

				input.push_frame(b);

				input.write_frame(
					b,
					positions[i].get_board(),
					c,
					positions[i].num_repetitions(),
					positions[i].move_number(),
					positions[i].halfmove_clock()
				);
			}
		}
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

int Search::color_to_move() {
	return positions[0].get_color_to_move();
}

std::string Search::get_name() {
	return nets[0].get_name();
}

std::vector<float> Search::get_lmm_frame() {
	int ctm = positions[0].get_color_to_move();
	return std::vector(inputs[ctm][0].get_lmm_input().begin(), inputs[ctm][0].get_lmm_input().begin() + 4096);
}

std::vector<float> Search::get_input_frame() {
	int ctm = positions[0].get_color_to_move();
	return std::vector(inputs[ctm][0].get_board_input().begin(), inputs[ctm][0].get_board_input().begin() + 8 * 8 * 85);
}
