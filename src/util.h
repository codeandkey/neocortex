/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "platform.h"

#include <iostream>
#include <chrono>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

namespace neocortex {
	namespace util {
		/**
		 * Formats a string.
		 * Behaves as printf() does, except the output is formatted to an std::string.
		 *
		 * @param fmt Format.
		 * @param args Parameters.
		 * @return Formatted string.
		 */
		template <typename ... Args>
		std::string format(std::string fmt, Args ... args) {
			int length = snprintf(nullptr, 0, fmt.c_str(), args ...) + 1;

			if (length <= 0) {
				throw std::runtime_error("Error occurred formatting string.");
			}

			char* buffer = new char[length];
			std::unique_ptr<char[]> buffer_ref(buffer);

			snprintf(buffer, length, fmt.c_str(), args ...);

			return std::string(buffer, buffer + length - 1);
		}

		/**
		 * Shorthand for formatting a runtime error.
		 *
		 * @param fmt Format string.
		 * @param args Format parameters.
		 * @return runtime_error exception with formatted string as content.
		 */
		template <typename ... Args>
		std::exception fmterr(std::string fmt, Args ... args) {
			return std::runtime_error(format(fmt, args...).c_str());
		}

		/* Time manipulation */
#ifdef NEOCORTEX_WIN32
		typedef clock_t time_point;
#else
		typedef struct timespec time_point;
#endif

		/**
		 * Gets a string representing the current time.
		 *
		 * @return Current time string.
		 */
		std::string timestring();

		/**
		 * Gets a reference to the current time.
		 *
		 * @return Current time point.
		 */
		time_point time_now();

		/**
		 * Gets the elapsed time in seconds since a reference point.
		 *
		 * @param reference Reference point.
		 * @return Seconds elapsed since the reference point.
		 */
		double time_elapsed(time_point reference);

		/**
		 * Gets the elapsed time in milliseconds since a reference point.
		 *
		 * @param reference Reference point.
		 * @return Milliseconds elapsed since the reference point.
		 */
		int time_elapsed_ms(time_point reference);

		/**
		 * Splits a string by a delimiter into a list of tokens.
		 *
		 * @param input Input string.
		 * @param delim Delimiter character.
		 * @return List of tokens in input, seperated by delimiter.
		 */
		std::vector<std::string> split(std::string input, char delim);

		/**
		 * Trims whitespace off of the head and tail of the input.
		 *
		 * @param input Input string.
		 * @return Trimmed input.
		 */
		std::string trim(std::string input);

		/* String joining */
		/**
		 * Joins a list of tokens together with a delimiter seperating them.
		 * Inverse of util::split().
		 *
		 * @param parts Input tokens.
		 * @param delim Delimiter.
		 *
		 * @return Joined string.
		 */
		std::string join(std::vector<std::string> parts, std::string delim);
	}
}
