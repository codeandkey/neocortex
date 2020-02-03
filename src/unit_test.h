#pragma once

/*
 * A unit test object.
 */

#include <string>

namespace nc2 {
    class UnitTest {
        public:
            UnitTest(std::string test_name);

            virtual void run() = 0;

            std::string get_name();

        private:
            std::string test_name;
    };
}
