#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "cat.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

static std::string copy(const std::string& data, std::size_t* n = nullptr) {
    std::istringstream in(data);
    std::ostringstream out;
    std::size_t copied = copy_stream(in, out);
    if (n != nullptr) {
        *n = copied;
    }
    return out.str();
}

TEST_CASE("copy_stream: zero bytes survive") {
    std::string data("a\0b\0\0c", 6);
    CHECK(copy(data) == data);
}

TEST_CASE("copy_stream: every byte value") {
    std::string data;
    for (int i = 0; i < 256; ++i) {
        data += static_cast<char>(i);
    }
    CHECK(copy(data) == data);
}

TEST_CASE("copy_stream: empty input") {
    std::size_t n = 99;
    CHECK(copy("", &n) == "");
    CHECK(n == 0);
}

TEST_CASE("copy_stream: bigger than the buffer") {
    std::string data(65536 * 3 + 17, 'x');
    data[65536] = '\n';
    std::size_t n = 0;
    CHECK(copy(data, &n) == data);
    CHECK(n == data.size());
}

TEST_CASE("run_cat: an empty file doesn't stop the next one") {
    write_file("empty.txt", "");
    write_file("next.txt", "still here");
    std::istringstream in;
    std::ostringstream out, err;
    CHECK(run_cat({"empty.txt", "next.txt"}, in, out, err) == 0);
    CHECK(out.str() == "still here");
}

TEST_CASE("run_cat: binary files concatenate exactly") {
    std::string a("\x89PNG\r\n\x1a\n", 8), b("\0\0\x01", 3);
    write_file("a.bin", a);
    write_file("b.bin", b);
    std::istringstream in;
    std::ostringstream out, err;
    run_cat({"a.bin", "b.bin"}, in, out, err);
    CHECK(out.str() == a + b);
}
