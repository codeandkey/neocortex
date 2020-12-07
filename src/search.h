/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "position.h"
#include "eval.h"
#include "move.h"
#include "util.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <iostream>
#include <thread>

namespace neocortex {
	namespace search {
		constexpr int QDEPTH = 4;
		constexpr int MAX_DEPTH = 128;
		constexpr int ALLOC_FRACTION = 10; /* use at most 1/nth of the remaining time */

		struct SearchInfo {
			int nodes = 0;
			int depth = 0;
			int score = 0;
			int time = 0;
			int seldepth = 0;
			PV pv;

			std::string to_string() {
				std::string out;

				out += "info depth " + std::to_string(depth) + " ";

				if (seldepth > 0) {
					out += "seldepth " + std::to_string(seldepth) + " ";
				}

				out += "score " + score::to_uci(score) + " ";
				out += "time " + std::to_string(time) + " ";
				out += "nodes " + std::to_string(nodes) + " ";

				if (time) {
					out += "nps " + std::to_string((long long) nodes * 1000 / time) + " ";
				}

				if (pv.len > 0) {
					out += "pv " + pv.to_string();
				}

				return out;
			}
		};

		class Search {
		public:
			/**
			 * Constructs a search.
			 *
			 * @param node Position to search.
			 */
			Search(Position node = Position());
			~Search();

			/**
			 * Starts searching in the background.
			 *
			 * @param parts List of arguments to UCI 'go'
			 * @param info Callback for search depth info. (optional)
			 * @param bestmove Callback for search bestmove. (optional)
			 * @param wtime White move time (ms)
			 * @param btime Black move time (ms)
			 * @param winc White increment (ms)
			 * @param binc Black increment (ms)
			 * @param depth Search depth
			 * @param movetime Move time (ms)
			 * @param infinite Infinite search
			 */
			void go(std::function<void(SearchInfo)> info, std::function<void(Move)> bestmove, int wtime = -1, int btime = -1, int winc = -1, int binc = -1, int depth = -1, int movetime = -1, bool infinite = false);

			/**
			 * Stops the running search.
			 */
			void stop();

			/**
			 * Waits for a running search to stop.
			 */
			void wait();

			/**
			 * Sets the number of threads in the search.
			 * Defaults to the number of processors.
			 *
			 * @param count Number of search threads.
			 */
			void set_threads(int count);

			/**
			 * Maximum threads that can be used for searching
			 * @return Maximum search threads.
			 */
			int max_threads();

			/**
			 * Loads a new position into the search.
			 *
			 * @param p Position to load.
			 */
			void load(Position p);

			/**
			 * Returns true if a search is active.
			 * @return true if search running, false otherwise
			 */
			bool is_running();

			/**
			 * Gets the current root position. Must not be called while the search is running.
			 * @return Current search root.
			 */
			Position get_position();
		private:
			/**
			 * Gets the elapsed time since the beginning of the whole search.
			 *
			 * @return Elapsed milliseconds.
			 */
			int elapsed();

			/**
			 * Gets the elapsed time since the beginning of the current depth.
			 *
			 * @return Elapsed milliseconds.
			 */
			int elapsed_iter();

			/**
			 * Returns true if there is an allocated time and it is expired.
			 *
			 * @return true if allocated time expired, false otherwise.
			 */
			bool allocated_time_expired();

		    /**
			 * Search control thread entry point.
			 * Started once for every 'go' command.
			 *
			 * @param out UCI output stream.
			 * @param info Callback for search depth info. (optional)
			 * @param bestmove Callback for search bestmove. (optional)
			 */
			void control_worker(Position root, std::function<void(SearchInfo)> info, std::function<void(Move)> bestmove);

			/**
			 * Search thread worker function.
			 *
			 * @param out UCI output stream.
			 * @param s_depth Depth to search.
			 * @param root Root position for worker.
			 */
			void smp_worker(int s_depth, Position root);

			/**
			 * Alpha-beta search routine.
			 * This function is called recursively throughout the search.
			 *
			 * @param root Position to search.
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param ply_dist Current ply distance from root.
			 * @param pv_line Output PV.
			 * @param abort_ref Abort flag to watch. Stops the search when set to true.
			 *
			 * @return Search score.
			 */
			int alphabeta(Position& root, int depth, int alpha, int beta, int ply_dist, PV* pv_line, std::atomic<bool>& abort_ref);

			/**
			 * Quiescence search routine.
			 * This search is called at the leaves of the normal alpha-beta tree.
			 * The quiescence search only examines winning captures and checks.
			 *
			 * @param root Position to search.
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param ply_dist Current ply distance from root.
			 * @param pv_line Output PV.
			 *
			 * @return Search score.
			 */
			int quiescence(Position& root, int depth, int alpha, int beta, int ply_dist, PV* pv_line);

			/**
			 * Quiescence capture search routine.
			 * This search is called at the leaves of the quiescence search tree.
			 * The quiescence search examines all captures until there are no more.
			 * If a check occurs with a capture then evasion movegen is called.
			 *
			 * @param root Position to search.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param ply_dist Current ply distance from root.
			 * @param pv_line Output PV.
			 *
			 * @return Search score.
			 */
			int quiescence_captures(Position& root, int alpha, int beta, int ply_dist, PV* pv_line);

			Position root;

			std::atomic<int> num_threads;

			std::vector<std::thread> smp_threads;
			std::atomic<bool> smp_should_stop;

			std::thread control_thread;
			std::atomic<bool> control_should_stop;

			std::mutex depth_starttime_mutex;
			util::time_point depth_starttime;

			/* Per-depth values */
			std::atomic<int> allocated_time; /* Time allocated to depth */
			std::atomic<int> max_ply_searched; /* Longest line searched (seldepth) */

			/* Search options */
			std::atomic<int> wtime, btime, winc, binc, movetime, depth;
			std::atomic<bool> infinite;
		};
	}
}
