#include "doctest.h"

#include "pixel.hpp"

// pixel.cpp is already written: these pass from the start. Your work is in
// CMakeLists.txt, which the variants check.

TEST_CASE("brighten") {
    CHECK(brighten(100, 40) == 140);
    CHECK(brighten(250, 40) == 255);
    CHECK(brighten(10, -40) == 0);
}
