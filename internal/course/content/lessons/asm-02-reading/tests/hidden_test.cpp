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

TEST_CASE("4. the first call to bump()") {
    CHECK(fnv(std::to_string(kFirstBump)) == 0x7ee7d07b4b19070u);
}

TEST_CASE("5. the function greet() calls") {
    // "_puts" and "puts@PLT" name the same function.
    std::string_view name = kGreetCalls;
    while (name.starts_with('_')) {
        name.remove_prefix(1);
    }
    name = name.substr(0, name.find('@'));
    CHECK(fnv(name) == 0x6c75180e275861c7u);
}
