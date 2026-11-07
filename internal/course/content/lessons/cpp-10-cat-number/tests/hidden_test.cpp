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

TEST_CASE("-n: empty lines get numbers too") {
    CHECK(cat({"-n"}, "a\n\nb\n") == "     1\ta\n     2\t\n     3\tb\n");
}

TEST_CASE("-n: numbering carries on into the next file") {
    write_file("n1.txt", "one\n");
    write_file("n2.txt", "two\nthree\n");
    CHECK(cat({"-n", "n1.txt", "n2.txt"}) == "     1\tone\n     2\ttwo\n     3\tthree\n");
}

TEST_CASE("-n: a file without a final newline continues its line") {
    write_file("part.txt", "no newline");
    write_file("rest.txt", " here\nnext\n");
    CHECK(cat({"-n", "part.txt", "rest.txt"}) == "     1\tno newline here\n     2\tnext\n");
}

TEST_CASE("-n: a last line with no newline is still numbered") {
    CHECK(cat({"-n"}, "a\nb") == "     1\ta\n     2\tb");
}

TEST_CASE("-b wins over -n") {
    CHECK(cat({"-bn"}, "a\n\nb\n") == "     1\ta\n\n     2\tb\n");
    CHECK(cat({"-n", "-b"}, "\nx\n") == "\n     1\tx\n");
}

TEST_CASE("-n: wide numbers push the tab along") {
    std::string in;
    for (int i = 0; i < 1000000; ++i) {
        in += '\n';
    }
    in += "last\n";
    std::string out = cat({"-n"}, in);
    CHECK(out.ends_with("999999\t\n1000000\t\n1000001\tlast\n"));
}

TEST_CASE("-n: empty input prints nothing") {
    CHECK(cat({"-n"}, "") == "");
}

TEST_CASE("no flags still copies exactly") {
    CHECK(cat({}, std::string("a\0b", 3)) == std::string("a\0b", 3));
}
