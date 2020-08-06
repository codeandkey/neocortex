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
			Search(Position node = Position());
			~Search();

			bool is_time_expired();
			void set_debug(bool enabled);

			void go(std::istream& in, std::ostream& out);
			void stop();

			void load(Position p);
		private:
			int elapsed();
			int elapsed_iter();
			void worker(std::ostream& out);

			int search_sync(int depth, int alpha, int beta, PV* pv_line);
			int alphabeta(int depth, int alpha, int beta, PV* pv_line);
			int quiescence(int depth, int alpha, int beta, PV* pv_line);

			std::thread search_thread;
			std::mutex search_mutex;

			Position root;
			std::atomic<bool> debug, should_stop;

			util::time_point search_starttime, depth_starttime;
			unsigned long numnodes;
		};
	}
}
