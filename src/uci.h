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