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
#include <list>
#include <mutex>
#include <iostream>
#include <thread>

namespace neocortex {
	namespace search {
		constexpr int QDEPTH = 4;
		constexpr int MAX_DEPTH = 128;
		constexpr int ALLOC_FRACTION = 10; /* use at most 1/nth of the remaining time */

		struct SearchStats {
			int num_nodes;
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
			 * Starts the search in the background.
			 *
			 * @param parts List of arguments to UCI 'go'
			 * @param out UCI output stream for 'info' and 'bestmove' messages
			 */
			void go(std::vector<std::string> parts, std::ostream& out);

			/**
			 * Stops the running search.
			 */
			void stop();

			/**
			 * Sets the number of threads in the search.
			 * Defaults to the number of processors.
			 *
			 * @param count Number of search threads.
			 */
			void set_threads(int count);

			/**
			 * Loads a new position into the search.
			 *
			 * @param p Position to load.
			 */
			void load(Position p);
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
			 * @param root Root position for worker.
			 */
			void control_worker(std::ostream& out, Position root);

			/**
			 * Search thread worker function.
			 *
			 * @param out UCI output stream.
			 * @param s_depth Depth to search.
			 * @param root Root position for worker.
			 */
			void smp_worker(std::ostream& out, int s_depth, Position root);

			/**
			 * Alpha-beta search routine.
			 * This function is called recursively throughout the search.
			 *
			 * @param root Position to search.
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param pv_line Output PV.
			 * @param nodes Optional output for number of nodes searched.
			 * @param abort_ref Abort flag to watch. Stops the search when set to true.
			 *
			 * @return Search score.
			 */
			int alphabeta(Position& root, int depth, int alpha, int beta, PV* pv_line, int* nodes_out, std::atomic<bool>& abort_ref);

			/**
			 * Quiescence search routine.
			 * This function is called selectively at the tail end of the search tree.
			 *
			 * @param root Position to search.
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param pv_line Output PV.
			 * @param nodes Optional output for number of nodes searched.
			 *
			 * @return Search score.
			 */
			int quiescence(Position& root, int depth, int alpha, int beta, PV* pv_line, int* nodes_out);

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
			std::atomic<int> num_nodes; /* Number of nodes searched */

			/* Search options */
			std::atomic<int> wtime, btime, winc, binc, movetime, depth;
			std::atomic<bool> infinite;
		};
	}
}
