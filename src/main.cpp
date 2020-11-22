/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "log.h"
#include "platform.h"
#include "uci.h"
#include "test.h"
#include "tt.h"
#include "zobrist.h"
#include "search.h"

#include <iostream>
#include <string>

using namespace neocortex;

int main(int argc, char** argv) {
#ifdef NEOCORTEX_DEBUG
	log::set_level(log::DEBUG);
#endif

	neocortex_info(NEOCORTEX_NAME " " NEOCORTEX_VERSION " " NEOCORTEX_BUILDTIME " " NEOCORTEX_DEBUG_STR "\n");

	zobrist::init();
	attacks::init();
	tt::init();

	if (argc > 1) {
		if (std::string("test") == std::string(argv[1])) {
#ifdef NDEBUG
			neocortex_error("Tests can only be run on debug builds.\n");
			return -1;
#else
			return Test::run_all();
#endif
		} else {
			neocortex_error("Unknown argument \"%s\"\n", argv[1]);
			neocortex_info("Available modes: debug, uci\n");
			return -1;
		}
	}

	try {
		uci::start(std::cin, std::cout);
	}
	catch (std::exception& e) {
		neocortex_error("Unhandled exception: %s\n", e.what());
	}

	return 0;
}
