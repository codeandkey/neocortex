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
		 * Initializes a UCI connection to a stream.
		 * Negotiates options and establishes the handshake.
		 *
		 * @param in Input stream.
		 * @param out Output stream.
		 */
		void connect(std::istream& in, std::ostream& out);

		/**
		 * Start listening for UCI commands on a stream.
		 * Returns only after UCI 'quit' is received or an error occurs.
		 *
		 * @param in Input stream.
		 * @param out Output stream.
		 */
		void begin(std::istream& in, std::ostream& out);

		/**
		 * Gets the uci hash size in MB.
		 *
		 * @return UCI_Hash value or default.
		 */
		int get_hash_size();
	}
}
