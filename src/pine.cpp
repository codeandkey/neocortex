/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "log.h"
#include "platform.h"
#include "uci.h"
#include "tt.h"
#include "zobrist.h"
#include "search.h"

#include <iostream>

using namespace neocortex;

int main(int argc, char** argv) {
#ifdef NEOCORTEX_DEBUG
	log::set_level(log::DEBUG);
#endif

	try {
		neocortex_debug("Starting neocortex %s\n", NEOCORTEX_VERSION);
		neocortex_debug("Build: %s\n", NEOCORTEX_BUILDTIME);
		neocortex_debug("Platform: %s\n", NEOCORTEX_PLATFORM);

#ifdef NEOCORTEX_DEBUG
		neocortex_warn("Compile time debug enabled. Performance will be slower!\n");
#endif

		uci::connect(std::cin, std::cout);

		zobrist::init();
		attacks::init();
		tt::init();

		uci::begin(std::cin, std::cout);
	}
	catch (std::exception& e) {
		neocortex_error("%s", e.what());
	}

	return 0;
}
