#pragma once

#include "platform.h"

#include <iostream>
#include <string>
#include <vector>

#define NC_TEST(name, action) static neocortex::test::Test name(#name, []() -> int action)

namespace neocortex {
	namespace test {
		class Test {
		public:
			Test(std::string name, int (*action)()) {
				this->name = name;
				this->action = action;

				tests().push_back(this);
			}

			/**
			 * Runs all available tests.
			 * 
			 * @return 0 if all tests passed, 1 otherwise.
			 */
			static int run_all() {
				int passed = 0, failed = 0, total = 0;

				std::cout << "Running neocortex tests..\n\n";

				for (auto t : tests()) {
					std::string reason;

					std::cout << t->name << " :: ";
					total += 1;

					try {
						if (t->action()) {
							reason = "Test returned non-zero exit status";
						}
					} catch (std::exception& e) {
						reason = e.what();
					}

					if (reason.size()) {
						failed += 1;
						std::cout << "failed: " << reason << "\n";
					} else {
						passed += 1;
						std::cout << "passed\n";
					}
				}

				std::cout << "\nresults: " << passed << " passed, " << failed << " failed, " << total << " total\n";
				return passed < total;
			}
		private:
			std::string name;
			int (*action)();

			static std::vector<Test*>& tests() {
				static std::vector<Test*> lst;
				return lst;
			}
		};
	}
}