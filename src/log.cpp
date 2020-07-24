#include "log.h"

using namespace pine::log;

std::recursive_mutex pine::log::log_mutex;

static int current_level = DEFAULT_LEVEL;
static ColorMode current_colormode = DEFAULT_COLORMODE;

void pine::log::set_level(int level) {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	current_level = level;
}

void pine::log::set_color(ColorMode mode) {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	current_colormode = mode;
}

int pine::log::get_level() {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	return current_level;
}

ColorMode pine::log::get_color() {
	std::lock_guard<std::recursive_mutex> lock(log_mutex);
	return current_colormode;
}