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