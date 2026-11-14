#include "doctest.h"
#include "tally.hpp"

TEST_CASE("stats has both counts") {
    Stats s = stats("a b\n");
    CHECK(s.words == 2);
    CHECK(s.lines == 1);
}
