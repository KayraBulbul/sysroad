#include "doctest.h"

#include <string>

#include "macros.hpp"

TEST_CASE("SQUARE") {
    CHECK(SQUARE(4) == 16);
    CHECK(SQUARE(1 + 2) == 9);
}

TEST_CASE("COUNT_OF") {
    int five[5] = {};
    CHECK(COUNT_OF(five) == 5);
}

TEST_CASE("STR expands macros first") {
    std::string line = STR(__LINE__);
    std::string want = std::to_string(__LINE__ - 1);  // the line above
    CHECK(line == want);
}

TEST_CASE("HERE") {
    std::string here = HERE;
    std::string want = std::string(__FILE__) + ":" + std::to_string(__LINE__ - 1);
    CHECK(here == want);
}

TEST_CASE("SHOW") {
    CHECK(SHOW(6 * 7) == "6 * 7 = 42");
}

TEST_CASE("arch and os") {
#if defined(__x86_64__)
    CHECK(std::string(arch()) == "x86-64");
#elif defined(__aarch64__)
    CHECK(std::string(arch()) == "arm64");
#endif
#if defined(__linux__)
    CHECK(std::string(os()) == "Linux");
#elif defined(__APPLE__)
    CHECK(std::string(os()) == "macOS");
#endif
}
