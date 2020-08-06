/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "log.h"

using namespace neocortex::log;

std::recursive_mutex neocortex::log::log_mutex;

static int current_level = DEFAULT_LEVEL;
static ColorMode current_colormode = DEFAULT_COLORMODE;

void neocortex::log::set_level(int level) {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	current_level = level;
}

void neocortex::log::set_color(ColorMode mode) {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	current_colormode = mode;
}

int neocortex::log::get_level() {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	return current_level;
}

ColorMode neocortex::log::get_color() {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	return current_colormode;
}