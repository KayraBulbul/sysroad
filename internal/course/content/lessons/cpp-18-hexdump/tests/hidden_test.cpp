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

TEST_CASE("hex_line: one byte") {
    CHECK(hex_line(0, "a") ==
          "00000000  61                                                |a|\n");
}

TEST_CASE("hex_line: non-printing and high bytes are dots") {
    CHECK(hex_line(0, std::string("\x00\x1f\x7f\x80\xff~ ", 7)) ==
          "00000000  00 1f 7f 80 ff 7e 20                              |.....~ |\n");
}

TEST_CASE("hex_line: exactly 8 bytes") {
    CHECK(hex_line(0x123, "ABCDEFGH") ==
          "00000123  41 42 43 44 45 46 47 48                           |ABCDEFGH|\n");
}

TEST_CASE("run_hexdump: empty input prints nothing") {
    CHECK(hexdump({}, "") == "");
}

TEST_CASE("run_hexdump: exactly 16 bytes") {
    CHECK(hexdump({}, "0123456789abcdef") ==
          "00000000  30 31 32 33 34 35 36 37  38 39 61 62 63 64 65 66  |0123456789abcdef|\n"
          "00000010\n");
}

TEST_CASE("Dumper: lines don't depend on how the input is cut up") {
    std::string data;
    for (int i = 0; i < 40; ++i) {
        data += static_cast<char>('a' + i % 26);
    }
    std::ostringstream whole, pieces;
    Dumper a(whole);
    a.feed(data);
    a.finish();
    Dumper b(pieces);
    for (char ch : data) {
        b.feed(std::string_view(&ch, 1));
    }
    b.finish();
    CHECK(pieces.str() == whole.str());
    CHECK(whole.str().ends_with("00000028\n"));
}

TEST_CASE("run_hexdump: files are one stream") {
    write_file("h1.bin", "abc");
    write_file("h2.bin", "def");
    CHECK(hexdump({"h1.bin", "h2.bin"}) ==
          "00000000  61 62 63 64 65 66                                 |abcdef|\n"
          "00000006\n");
}

TEST_CASE("run_hexdump: a missing file") {
    write_file("ok.bin", "ok");
    std::istringstream in;
    std::ostringstream out, err;
    int status = run_hexdump({"nope.bin", "ok.bin"}, in, out, err);
    CHECK(err.str() == "hexdump: nope.bin: No such file or directory\n");
    CHECK(out.str() == "00000000  6f 6b                                             |ok|\n"
                       "00000002\n");
    CHECK(status == 1);
}

TEST_CASE("run_hexdump: offsets past 0xffffffff keep growing") {
    CHECK(hex_line(0x1'0000'0000, "x").starts_with("100000000  78 "));
}
