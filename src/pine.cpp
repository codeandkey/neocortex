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

using namespace pine;

int main(int argc, char** argv) {
#ifdef PINE_DEBUG
	log::set_level(log::DEBUG);
#endif

	try {
		pine_debug("Starting pine %s\n", PINE_VERSION);
		pine_debug("Build: %s\n", PINE_BUILDTIME);
		pine_debug("Platform: %s\n", PINE_PLATFORM);

#ifdef PINE_DEBUG
		pine_warn("Compile time debug enabled. Performance will be slower!\n");
#endif

		uci::connect(std::cin, std::cout);

		zobrist::init();
		attacks::init();
		tt::init();

		uci::begin(std::cin, std::cout);
	}
	catch (std::exception& e) {
		pine_error("%s", e.what());
	}

	return 0;
}
