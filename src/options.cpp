#include "options.h"
#include "log.h"
#include "util.h"

#include <mutex>

using namespace pine;

static std::mutex option_mutex;
static std::map<std::string, options::Value> option_values;

options::Value::Value(int intval) : type(STRING), int_val(intval), bool_val(false) {}
options::Value::Value(bool boolval) : type(STRING), int_val(0), bool_val(boolval) {}
options::Value::Value(std::string strval) : type(STRING), int_val(0), bool_val(false), str_val(strval) {}

options::Value::operator std::string() {
	if (type != STRING) {
		throw util::fmterr("Stored option is not a string");
	}

	return str_val;
}

options::Value::operator int() {
	if (type != INTEGER) {
		throw util::fmterr("Stored option is not an integer");
	}

	return int_val;
}

options::Value::operator bool() {
	if (type != BOOLEAN) {
		throw util::fmterr("Stored option is not a boolean");
	}

	return bool_val;
}

template <typename T>
void options::set(std::string key, T val) {
	std::lock_guard<std::mutex> lock(option_mutex);
	option_values[key] = val;
}

template <typename T>
T options::get(std::string key, T def) {
	std::lock_guard<std::mutex> lock(option_mutex);
	
	try {
		return option_values.at(key);
	}
	catch (std::out_of_range) {}
	catch (std::exception& e) {
		pine_warn("Failed retrieving option %s: %s", key.c_str(), e.what());
	}

	return def;
}
