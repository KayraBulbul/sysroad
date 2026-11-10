#include "doctest.h"

#include <cstdint>
#include <string>
#include <string_view>

#include "answers.hpp"

static std::uint64_t fnv(std::string_view s) {
    std::uint64_t h = 0xcbf29ce484222325;
    for (unsigned char c : s) {
        h = (h ^ c) * 0x100000001b3;
    }
    return h;
}

TEST_CASE("phase 3: the index") {
    CHECK(fnv(std::to_string(kIndex)) == 0xaf63a94c860195e3u);
}
