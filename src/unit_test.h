#pragma once

/*
 * A unit test object.
 */

#include <string>

namespace nc2 {
    class UnitTest {
        public:
            /**
             * Initialize a unit test.
             *
             * @param test_name Name for test.
             */
            UnitTest(std::string test_name);

            /**
             * Runs the unit test.
             * Should raise an exception if the test fails.
             */
            virtual void run() = 0;

            /**
             * Gets the name of this test.
             *
             * @return Test name
             */
            std::string get_name();

        private:
            std::string test_name;
    };
}
