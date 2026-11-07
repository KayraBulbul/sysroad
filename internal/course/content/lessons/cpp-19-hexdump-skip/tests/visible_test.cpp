#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "hexdump.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

static std::string hexdump(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    run_hexdump(args, in, out, err);
    return out.str();
}

static const std::string zero_line =
    "00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|\n";

TEST_CASE("repeated lines squeeze into *") {
    CHECK(hexdump({"-C"}, std::string(64, '\0')) == zero_line + "*\n00000040\n");
}

TEST_CASE("-s skips, and offsets count the skipped bytes") {
    CHECK(hexdump({"-C", "-s", "4"}, "Hello world\n") ==
          "00000004  6f 20 77 6f 72 6c 64 0a                           |o world.|\n"
          "0000000c\n");
}

TEST_CASE("-n stops early") {
    CHECK(hexdump({"-n", "5"}, "Hello world\n") ==
          "00000000  48 65 6c 6c 6f                                    |Hello|\n"
          "00000005\n");
}
