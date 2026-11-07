#include "doctest.h"

int add(int a, int b);

TEST_CASE("add: small numbers") {
    CHECK(add(2, 3) == 5);
}
