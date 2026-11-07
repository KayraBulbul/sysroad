#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "cat.hpp"

static std::string copy(const std::string& data, std::size_t* n = nullptr) {
    std::istringstream in(data);
    std::ostringstream out;
    std::size_t copied = copy_stream(in, out);
    if (n != nullptr) {
        *n = copied;
    }
    return out.str();
}

TEST_CASE("copy_stream: text") {
    std::size_t n = 0;
    CHECK(copy("one\ntwo\n", &n) == "one\ntwo\n");
    CHECK(n == 8);
}

TEST_CASE("copy_stream: no newline at the end") {
    CHECK(copy("no newline") == "no newline");
}
