#include "doctest.h"

#include <string>

#include "macros.hpp"

TEST_CASE("SQUARE in bigger expressions") {
    CHECK(SQUARE(1 + 2) * 2 == 18);
    CHECK(100 / SQUARE(5) == 4);
    CHECK(-SQUARE(3) == -9);
    int x = 3;
    CHECK(SQUARE(x - 5) == 4);
}

TEST_CASE("COUNT_OF of other types and in expressions") {
    double d[7] = {};
    char s[] = "hello";  // 5 letters and a '\0'
    struct Big {
        char bytes[100];
    } big[3] = {};
    CHECK(COUNT_OF(d) == 7);
    CHECK(COUNT_OF(s) == 6);
    CHECK(COUNT_OF(big) == 3);
    CHECK(COUNT_OF(d) * 2 == 14);
}

#define ANSWER 42
TEST_CASE("STR of a macro and of plain text") {
    CHECK(std::string(STR(ANSWER)) == "42");
    CHECK(std::string(STR(hello world)) == "hello world");
}

TEST_CASE("HERE is a string literal") {
    const char* here = "at " HERE;  // only a literal can be glued to another like this
    CHECK(std::string(here).starts_with("at "));
    CHECK(std::string(here).ends_with(":" + std::to_string(__LINE__ - 2)));
}

TEST_CASE("SHOW evaluates its expression once, with the right grouping") {
    int calls = 0;
    auto f = [&] {
        ++calls;
        return 5;
    };
    CHECK(SHOW(f() + 1) == "f() + 1 = 6");
    CHECK(calls == 1);
    CHECK(SHOW(1 + 2) == "1 + 2 = 3");
    long long big = 1LL << 40;
    CHECK(SHOW(big) == "big = 1099511627776");
}
