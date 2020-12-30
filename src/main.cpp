/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "log.h"
#include "platform.h"
#include "uci.h"
#include "pht.h"
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

	bb::init();
	zobrist::init();
	attacks::init();
	tt::init();
	pht::init();

	try {
		uci::start();
	}
	catch (std::exception& e) {
		neocortex_error("Unhandled exception: %s\n", e.what());
	}

	return 0;
}
