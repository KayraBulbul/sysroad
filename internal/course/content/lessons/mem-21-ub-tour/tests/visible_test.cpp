#include "doctest.h"

#include <climits>
#include <vector>

#include "ub.hpp"

TEST_CASE("midpoint near the top") {
    CHECK(midpoint(INT_MAX, INT_MAX - 2) == INT_MAX - 1);
}

TEST_CASE("rotl by 0 and 8") {
    CHECK(rotl(0x12345678, 8) == 0x34567812);
    CHECK(rotl(0x12345678, 0) == 0x12345678);
}

TEST_CASE("read_u32 at an odd address") {
    alignas(4) unsigned char bytes[8] = {0, 0x78, 0x56, 0x34, 0x12, 0, 0, 0};
    CHECK(read_u32(bytes + 1) == 0x12345678);
}

TEST_CASE("has_adjacent_equal") {
    CHECK_FALSE(has_adjacent_equal({1, 2, 3, 4}));
    CHECK(has_adjacent_equal({1, 2, 2, 4}));
}

TEST_CASE("magnitude of INT_MIN") {
    CHECK(magnitude(INT_MIN) == 2147483648u);
}
