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

TEST_CASE("3. the line that crashes") {
    CHECK(fnv(std::to_string(kCrashLine)) == 0x7eb0207b4ae8998u);
}

TEST_CASE("4. the name it crashed on") {
    CHECK(fnv(kMissingName) == 0x40907fff355efc33u);
}
