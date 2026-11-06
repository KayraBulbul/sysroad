#include "doctest.h"

int add(int a, int b);

TEST_CASE("add: negatives") {
    CHECK(add(-4, 4) == 0);
    CHECK(add(-2, -3) == -5);
}

TEST_CASE("add: zero") {
    CHECK(add(0, 0) == 0);
}
