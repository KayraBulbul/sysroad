#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "wc.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

struct Ran {
    int status;
    std::string out, err;
};

static Ran wc(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    int status = run_wc(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST_CASE("run_wc: two files and a total") {
    write_file("h1.txt", "hello world\n");
    write_file("h2.txt", "one\ntwo three\n");
    Ran r = wc({"h1.txt", "h2.txt"});
    CHECK(r.out == "      1       2      12 h1.txt\n"
                   "      2       3      14 h2.txt\n"
                   "      3       5      26 total\n");
    CHECK(r.status == 0);
}

TEST_CASE("run_wc: a missing file in the middle") {
    write_file("m1.txt", "a\n");
    write_file("m2.txt", "b c\n");
    Ran r = wc({"m1.txt", "missing.txt", "m2.txt"});
    CHECK(r.out == "      1       1       2 m1.txt\n"
                   "      1       2       4 m2.txt\n"
                   "      2       3       6 total\n");
    CHECK(r.err == "wc: missing.txt: No such file or directory\n");
    CHECK(r.status == 1);
}

TEST_CASE("run_wc: - reads stdin, among files") {
    write_file("d1.txt", "x y z\n");
    Ran r = wc({"d1.txt", "-"}, "from stdin\n");
    CHECK(r.out == "      1       3       6 d1.txt\n"
                   "      1       2      11 -\n"
                   "      2       5      17 total\n");
}

TEST_CASE("run_wc: flags apply to every line") {
    write_file("f1.txt", "one\ntwo\n");
    write_file("f2.txt", "three\n");
    Ran r = wc({"-l", "f1.txt", "f2.txt"});
    CHECK(r.out == "      2 f1.txt\n"
                   "      1 f2.txt\n"
                   "      3 total\n");
}

TEST_CASE("run_wc: total prints even when every file failed") {
    Ran r = wc({"gone1", "gone2"});
    CHECK(r.out == "      0       0       0 total\n");
    CHECK(r.status == 1);
}

TEST_CASE("run_wc: a bad flag") {
    Ran r = wc({"-k"});
    CHECK(r.err == "wc: invalid option -- 'k'\n");
    CHECK(r.status == 1);
}

TEST_CASE("run_wc: binary files count every byte") {
    std::string data("a\0b\n\0", 5);
    write_file("bin.dat", data);
    Ran r = wc({"-c", "bin.dat"});
    CHECK(r.out == "      5 bin.dat\n");
}
