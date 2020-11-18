/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

namespace neocortex {
    class Test {
        public:
            Test(std::string name, void (*action)()) {
                this->name = name;
                this->action = action;

                tests().push_back(this);
            };

            static int run_all() {
                int passed = 0, failed = 0, total = 0;

                for (auto i : tests()) {
                    try {
						++total;
                        i->action();
                        ++passed;
                        neocortex_debug("%s: passed\n", i->name.c_str());
                    } catch (std::exception& e) {
                        neocortex_error("%s: failed: %s\n", i->name.c_str(), e.what());
                        ++failed;
                    }
                }
                
                neocortex_debug("%d / %d tests passed\n", passed, total);

                if (failed) {
                    neocortex_error("%d failures\n", failed);
					return -1;
                } else {
					return 0;
				}
            }

        private:
            std::string name;
            void (*action)();

            static std::vector<Test*>& tests() {
                static std::vector<Test*> _tests;
                return _tests;
            }
    };
}

#define NC_TEST(name) static void nc_test_fun_ ## name (); static neocortex::Test nc_test ## name (#name, nc_test_fun_ ## name ); void nc_test_fun_ ## name ()

#define NC_ASSERT(cond) if (!(cond)) { throw std::runtime_error("Assertion failed: " #cond); }
#define NC_ASSERT_EQ(a, b) NC_ASSERT((A) == (B))
