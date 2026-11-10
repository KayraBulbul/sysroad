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

TEST_CASE("phase 1: the word") {
    CHECK(fnv(kWord) == 0x35cef65cea523486u);
}

TEST_CASE("phase 2: the number") {
    CHECK(fnv(std::to_string(kNumber)) == 0x6eaccd922ff230f7u);
}
