/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "platform.h"

#include <iostream>

namespace pine {
	namespace uci {
		constexpr const char* AUTHOR = "codeandkey";
		constexpr const char* NAME = PINE_NAME " " PINE_VERSION;

		void connect(std::istream& in, std::ostream& out);
		void begin(std::istream& in, std::ostream& out);
	}
}