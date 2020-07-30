/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <map>
#include <string>

namespace pine {
	namespace options {
		class Value {
		public:
			Value(std::string str);
			Value(int integer);
			Value(bool boolean);

			operator std::string();
			operator int();
			operator bool();

			static constexpr int STRING = 0;
			static constexpr int INTEGER = 1;
			static constexpr int BOOLEAN = 2;
		private:
			int type;
			int int_val;
			bool bool_val;
			std::string str_val;
		};

		template <typename T> void set(std::string key, T value);
		template <typename T> T get(std::string key, T def);
	}
}