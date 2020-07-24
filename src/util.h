#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <stdexcept>
#include <vector>

namespace pine {
	namespace util {
		/* String formatting */
		template <typename ... Args>
		std::string format(std::string fmt, Args ... args) {
			int length = snprintf(nullptr, 0, fmt.c_str(), args ...) + 1;

			if (length <= 0) {
				throw std::runtime_error("Error occurred formatting string.");
			}

			char* buffer = new char[length];
			std::unique_ptr<char> buffer_ref(buffer);

			snprintf(buffer, length, fmt.c_str(), args ...);

			return std::string(buffer, buffer + length - 1);
		}

		/* Exception shorthand */
		template <typename ... Args>
		std::exception fmterr(std::string fmt, Args ... args) {
			return std::runtime_error(format(fmt, args...).c_str());
		}

		/* Time manipulation */
		typedef std::chrono::steady_clock::time_point time_point;
		std::string timestring();
		time_point now();
		double elapsed(time_point reference);
		int elapsed_ms(time_point reference);

		/* String splitting */
		std::vector<std::string> split(std::string input, char delim);

		/* String trimming */
		std::string trim(std::string input);

		/* String joining */
		std::string join(std::vector<std::string> parts, std::string delim);
	}
}