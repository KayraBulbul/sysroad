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

static std::string without_spaces(std::string_view s) {
    std::string out;
    for (char c : s) {
        if (c != ' ') {
            out += c;
        }
    }
    return out;
}

TEST_CASE("1. the first function, demangled") {
    CHECK(fnv(without_spaces(kFirstFunction)) == 0x3d3bcbd3a887e74eu);
}

TEST_CASE("2. the text banner() returns") {
    CHECK(fnv(kBanner) == 0x541b64144641ff40u);
}

TEST_CASE("3. the number answer() returns") {
    CHECK(fnv(std::to_string(kAnswer)) == 0x3e0f8e181452374cu);
}
