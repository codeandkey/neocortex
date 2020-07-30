/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <string>

namespace pine {
	namespace square {
		constexpr int null = -1;

		int at(int rank, int file);
		int from_uci(std::string uci);

		int rank(int sq);
		int file(int sq);

		std::string to_uci(int sq);
		bool is_valid(int sq);
	}
}