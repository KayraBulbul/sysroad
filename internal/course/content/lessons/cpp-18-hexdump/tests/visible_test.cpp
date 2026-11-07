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

TEST_CASE("hex_line: a short line") {
    CHECK(hex_line(0, "Hello world\n") ==
          "00000000  48 65 6c 6c 6f 20 77 6f  72 6c 64 0a              |Hello world.|\n");
}

TEST_CASE("hex_line: a full line") {
    CHECK(hex_line(16, "0123456789abcdef") ==
          "00000010  30 31 32 33 34 35 36 37  38 39 61 62 63 64 65 66  |0123456789abcdef|\n");
}

TEST_CASE("run_hexdump: stdin") {
    CHECK(hexdump({"-C"}, "Hello world\n") ==
          "00000000  48 65 6c 6c 6f 20 77 6f  72 6c 64 0a              |Hello world.|\n"
          "0000000c\n");
}
