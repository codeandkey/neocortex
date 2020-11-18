/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "platform.h"

#include <iostream>

namespace neocortex {
	namespace uci {
		constexpr const char* AUTHOR = "Justin Stanley";
		constexpr const char* NAME = NEOCORTEX_NAME " " NEOCORTEX_VERSION;

		/**
		 * Starts the UCI interface.
		 * Returns only after UCI 'quit' is received or an error occurs.
		 *
		 * @param in Input stream.
		 * @param out Output stream.
		 */
		void start(std::istream& in, std::ostream& out);
	}
}
