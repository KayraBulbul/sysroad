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

TEST_CASE("squeezing: the next different line keeps its offset") {
    std::string data = std::string(48, '\0') + "abc";
    CHECK(hexdump({}, data) == zero_line + "*\n" +
                                   "00000030  61 62 63                                          |abc|\n"
                                   "00000033\n");
}

TEST_CASE("squeezing: a partial last line of the same bytes still prints") {
    std::string data(40, '\0');
    CHECK(hexdump({}, data) == zero_line + "*\n" +
                                   "00000020  00 00 00 00 00 00 00 00                           |........|\n"
                                   "00000028\n");
}

TEST_CASE("squeezing: only lines next to each other") {
    std::string a(16, 'A'), b(16, 'B');
    std::string out = hexdump({}, a + b + a + a + a);
    CHECK(out ==
          "00000000  41 41 41 41 41 41 41 41  41 41 41 41 41 41 41 41  |AAAAAAAAAAAAAAAA|\n"
          "00000010  42 42 42 42 42 42 42 42  42 42 42 42 42 42 42 42  |BBBBBBBBBBBBBBBB|\n"
          "00000020  41 41 41 41 41 41 41 41  41 41 41 41 41 41 41 41  |AAAAAAAAAAAAAAAA|\n"
          "*\n"
          "00000050\n");
}

TEST_CASE("-v prints every line") {
    std::string out = hexdump({"-Cv"}, std::string(32, '\0'));
    CHECK(out == zero_line +
                     "00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|\n"
                     "00000020\n");
}

TEST_CASE("-s: stuck-on numbers and hex") {
    CHECK(hexdump({"-s16"}, "0123456789abcdefXYZ") ==
          "00000010  58 59 5a                                          |XYZ|\n00000013\n");
    CHECK(hexdump({"-s", "0x10"}, "0123456789abcdefXYZ") ==
          "00000010  58 59 5a                                          |XYZ|\n00000013\n");
}

TEST_CASE("-s past the end prints just the length") {
    CHECK(hexdump({"-s", "10"}, "abc") == "00000003\n");
}

TEST_CASE("-s across a chunk boundary and two files") {
    write_file("p1.bin", std::string(5000, 'x'));
    write_file("p2.bin", "tail");
    CHECK(hexdump({"-s", "5002", "p1.bin", "p2.bin"}) ==
          "0000138a  69 6c                                             |il|\n0000138c\n");
}

TEST_CASE("-s then -n") {
    CHECK(hexdump({"-s", "4", "-n", "10"}, "Hello world, this is long\n") ==
          "00000004  6f 20 77 6f 72 6c 64 2c  20 74                    |o world, t|\n"
          "0000000e\n");
}

TEST_CASE("-n 0 prints nothing") {
    CHECK(hexdump({"-n", "0"}, "abc") == "");
    CHECK(hexdump({"-s", "1", "-n", "0"}, "abc") == "");
}

TEST_CASE("-n larger than the input") {
    CHECK(hexdump({"-n", "100"}, "ab") ==
          "00000000  61 62                                             |ab|\n00000002\n");
}

TEST_CASE("bad arguments") {
    std::istringstream in("x");
    std::ostringstream out, err;
    CHECK(run_hexdump({"-s", "12abc"}, in, out, err) == 1);
    CHECK(err.str() == "hexdump: invalid number '12abc'\n");
}
