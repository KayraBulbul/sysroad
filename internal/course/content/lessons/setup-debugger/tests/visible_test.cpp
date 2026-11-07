#include "doctest.h"

#include <cstdint>
#include <string>
#include <string_view>

#include "answers.hpp"

// The tests compare a hash of your answer, so a failure doesn't give the
// right one away.
static std::uint64_t fnv(std::string_view s) {
    std::uint64_t h = 0xcbf29ce484222325;
    for (unsigned char c : s) {
        h = (h ^ c) * 0x100000001b3;
    }
    return h;
}

TEST_CASE("1. walk(1, 1000)") {
    CHECK(fnv(std::to_string(kWalk1000)) == 0x3b83aa84ebbca43cu);
}

TEST_CASE("2. acc when i == 37 in checksum(100)") {
    CHECK(fnv(std::to_string(kAccAt37)) == 0x27e5ca5bd3424cb1u);
}
