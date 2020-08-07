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
		constexpr int PV_MAX = 128;
		constexpr int ALLOC_FRACTION = 10; /* use at most 1/nth of the remaining time */

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
			 * Test if the alloacted search time has expired, if the search is not infinite.
			 *
			 * @return true if allocated time is expired, false otherwise.
			 */
			bool is_time_expired();

			/**
			 * Sets the search debug mode.
			 *
			 * @param enabled Enable flag.
			 */
			void set_debug(bool enabled);

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
			 * Search thread worker function.
			 *
			 * @param out UCI output stream.
			 */
			void worker(std::ostream& out);

			/**
			 * Run a search.
			 * This function is called once per search and initializes the time control points.
			 *
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param pv_line Output PV.
			 *
			 * @return Search score.
			 */
			int search_sync(int depth, int alpha, int beta, PV* pv_line);

			/**
			 * Alpha-beta search routine.
			 * This function is called recursively throughout the search.
			 *
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param pv_line Output PV.
			 *
			 * @return Search score.
			 */
			int alphabeta(int depth, int alpha, int beta, PV* pv_line);

			/**
			 * Quiescence search routine.
			 * This function is called selectively at the tail end of the search tree.
			 *
			 * @param depth Search depth.
			 * @param alpha Search alpha.
			 * @param beta Search beta.
			 * @param pv_line Output PV.
			 *
			 * @return Search score.
			 */
			int quiescence(int depth, int alpha, int beta, PV* pv_line);

			std::thread search_thread;
			std::mutex search_mutex;

			Position root;
			std::atomic<bool> debug, should_stop;

			util::time_point search_starttime, depth_starttime;
			std::atomic<int> wtime, btime, winc, binc, movetime, allocated_time, nodes, numnodes, depth;
			std::atomic<bool> infinite;
		};
	}
}
