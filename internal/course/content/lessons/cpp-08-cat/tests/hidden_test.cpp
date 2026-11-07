#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "cat.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

struct Ran {
    int status;
    std::string out, err;
};

static Ran cat(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    int status = run_cat(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST_CASE("run_cat: - is stdin, between files") {
    write_file("x.txt", "x\n");
    write_file("z.txt", "z\n");
    Ran r = cat({"x.txt", "-", "z.txt"}, "y\n");
    CHECK(r.out == "x\ny\nz\n");
}

TEST_CASE("run_cat: a missing file is reported, the rest still print") {
    write_file("ok.txt", "fine\n");
    Ran r = cat({"missing.txt", "ok.txt"});
    CHECK(r.out == "fine\n");
    CHECK(r.err == "cat: missing.txt: No such file or directory\n");
    CHECK(r.status == 1);
}

TEST_CASE("run_cat: a directory") {
    std::filesystem::create_directory("somedir");
    write_file("after.txt", "after\n");
    Ran r = cat({"somedir", "after.txt"});
    CHECK(r.out == "after\n");
    CHECK(r.err == "cat: somedir: Is a directory\n");
    CHECK(r.status == 1);
}

TEST_CASE("run_cat: empty files and empty lines") {
    write_file("empty.txt", "");
    write_file("blanks.txt", "\n\nx\n\n");
    Ran r = cat({"empty.txt", "blanks.txt", "empty.txt"});
    CHECK(r.out == "\n\nx\n\n");
    CHECK(r.status == 0);
}
