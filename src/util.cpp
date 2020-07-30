/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "util.h"
#include "platform.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

#if defined PINE_LINUX || defined PINE_APPLE
#include <errno.h>
#include <cstring>
#endif

using namespace pine;

std::string util::timestring() {
	std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm tm;
	
#ifdef PINE_WIN32
	if (localtime_s(&tm, &t)) {
		throw std::runtime_error("localtime() failed.");
	}
#else
	if (!localtime_r(&t, &tm)) {
		throw std::runtime_error("localtime() failed.");
	}
#endif

	std::stringstream ss;
	ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

	return ss.str();
}

util::time_point util::now() {
	return std::chrono::steady_clock::now();
}

double util::elapsed(time_point reference) {
	return std::chrono::duration_cast<std::chrono::microseconds>(pine::util::now() - reference).count() / 1000000.0;
}

int util::elapsed_ms(time_point reference) {
	return (int) std::chrono::duration_cast<std::chrono::milliseconds>(pine::util::now() - reference).count();
}

std::vector<std::string> util::split(std::string input, char delim) {
	char* buf = new char[input.size() + 1];
	int token_ind = 0;

	std::vector<std::string> result;

	for (size_t c = 0; c < input.size(); ++c) {
		if (input[c] == delim) {
			if (token_ind > 0) {
				buf[token_ind] = '\0';
				token_ind = 0;
				result.push_back(std::string(buf));
			} else {
				continue;
			}
		} else {
			buf[token_ind++] = input[c];
		}
	}

	if (token_ind > 0) {
		buf[token_ind] = '\0';
		token_ind = 0;
		result.push_back(std::string(buf));
	}

	return result;
}

std::string util::trim(std::string input) {
	input.erase(input.begin(), std::find_if_not(input.begin(), input.end(), [](char c) { return std::isspace(c); }));
	input.erase(std::find_if_not(input.rbegin(), input.rend(), [](char c) { return std::isspace(c); }).base(), input.end());

	return input;
}

std::string util::join(std::vector<std::string> parts, std::string delim) {
	std::string output;

	for (size_t i = 0; i < parts.size(); ++i) {
		if (i) output += delim;
		output += parts[i];
	}

	return output;
}
