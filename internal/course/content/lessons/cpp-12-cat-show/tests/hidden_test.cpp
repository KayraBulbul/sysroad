#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "cat.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

static std::string cat(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    run_cat(args, in, out, err);
    return out.str();
}

TEST_CASE("-v: DEL") {
    CHECK(cat({"-v"}, "\x7f\n") == "^?\n");
}

TEST_CASE("-v: tabs and newlines pass through") {
    CHECK(cat({"-v"}, "a\tb\n") == "a\tb\n");
}

TEST_CASE("-v: bytes with the high bit set") {
    CHECK(cat({"-v"}, "\x80\xff\n") == "M-^@M-^?\n");
    CHECK(cat({"-v"}, "\xc2\xa0\x9b") == "M-BM- M-^[");
}

TEST_CASE("-v: zero bytes") {
    CHECK(cat({"-v"}, std::string("a\0b", 3)) == "a^@b");
}

TEST_CASE("-v: \\r shows as ^M") {
    CHECK(cat({"-v"}, "dos line\r\n") == "dos line^M\n");
}

TEST_CASE("-A is -vET") {
    CHECK(cat({"-A"}, "x\x01\t\r\n") == "x^A^I^M$\n");
}

TEST_CASE("-e is -vE, -t is -vT") {
    CHECK(cat({"-e"}, "\x02\t\n") == "^B\t$\n");
    CHECK(cat({"-t"}, "\x02\t\n") == "^B^I\n");
}

TEST_CASE("-T without -v leaves other bytes alone") {
    CHECK(cat({"-T"}, "\x01\t\n") == "\x01^I\n");
}

TEST_CASE("-nE together") {
    CHECK(cat({"-nE"}, "a\n\n") == "     1\ta$\n     2\t$\n");
}

TEST_CASE("-E on a line with no newline") {
    CHECK(cat({"-E"}, "end") == "end");
}

TEST_CASE("-sE squeezes, then marks") {
    CHECK(cat({"-sE"}, "a\n\n\n\nb\n") == "a$\n$\nb$\n");
}

TEST_CASE("-E: \\r before a newline is ^M") {
    CHECK(cat({"-E"}, "dos\r\n") == "dos^M$\n");
}

TEST_CASE("-E: other \\r bytes stay as they are") {
    CHECK(cat({"-E"}, "a\rb\n") == "a\rb$\n");
    CHECK(cat({"-E"}, "x\r\r\n") == "x\r^M$\n");
    CHECK(cat({"-E"}, "end\r") == "end\r");
}

TEST_CASE("-E: the \\r and \\n in different files") {
    write_file("cr.txt", "a\r");
    write_file("lf.txt", "\nb\n");
    CHECK(cat({"-E", "cr.txt", "lf.txt"}) == "a^M$\nb$\n");
}

TEST_CASE("-E: \\r\\n across a 4096-byte read") {
    std::string in(4095, 'x');
    in += "\r\n";
    CHECK(cat({"-E"}, in) == std::string(4095, 'x') + "^M$\n");
}
