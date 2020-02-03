#pragma once

/*
 * Dummy test.
 */

#include "../unit_test.h"

namespace nc2 {
    namespace tests {
        class DummyTest : public UnitTest {
        public:
            DummyTest();

            void run();
        };
    }
}
