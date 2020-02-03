#include "unit_test.h"

using namespace nc2;

UnitTest::UnitTest(std::string test_name) : test_name(test_name) {}

std::string UnitTest::get_name() {
    return test_name;
}
