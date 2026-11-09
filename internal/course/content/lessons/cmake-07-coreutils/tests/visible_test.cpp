#include "doctest.h"

#include <istream>
#include <string>
#include <vector>

#include "coreutils/io.hpp"

TEST_CASE("for_each_input visits files in order") {
    // A file that isn't there is reported and skipped.
    std::vector<std::string> seen;
    char a[] = "no-such-file-1";
    char* files[] = {a};
    int status = coreutils::for_each_input(
        "t", files, [&](std::istream&, std::string_view name) { seen.emplace_back(name); });
    CHECK(status == 1);
    CHECK(seen.empty());
}
