#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "grep.hpp"

namespace fs = std::filesystem;

static void write_file(const std::string& name, const std::string& data) {
    if (fs::path dir = fs::path(name).parent_path(); !dir.empty()) {
        fs::create_directories(dir);
    }
    std::ofstream(name, std::ios::binary) << data;
}

struct Ran {
    int status;
    std::string out, err;
};

static Ran grep(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    int status = run_grep(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST_CASE("-r searches a tree, in sorted order") {
    write_file("t1/b.txt", "hit b\n");
    write_file("t1/a.txt", "hit a\nmiss\n");
    write_file("t1/sub/c.txt", "hit c\n");
    Ran r = grep({"-r", "hit", "t1"});
    CHECK(r.out == "t1/a.txt:hit a\nt1/b.txt:hit b\nt1/sub/c.txt:hit c\n");
    CHECK(r.status == 0);
}

TEST_CASE("-rn") {
    write_file("t2/x.cpp", "a\n// TODO\n");
    CHECK(grep({"-rn", "TODO", "t2"}).out == "t2/x.cpp:2:// TODO\n");
}
