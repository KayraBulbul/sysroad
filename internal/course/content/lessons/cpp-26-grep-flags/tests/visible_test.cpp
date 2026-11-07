#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "grep.hpp"

static void write_file(const std::string& name, const std::string& data) {
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

TEST_CASE("-i ignores case") {
    CHECK(grep({"-i", "todo"}, "TODO a\ntodo b\nToDo c\ndone\n").out == "TODO a\ntodo b\nToDo c\n");
}

TEST_CASE("-v selects the other lines") {
    CHECK(grep({"-v", "#"}, "# comment\ncode\n# more\nend\n").out == "code\nend\n");
}

TEST_CASE("-n numbers lines") {
    CHECK(grep({"-n", "b"}, "a\nb\nc\nab\n").out == "2:b\n4:ab\n");
}

TEST_CASE("-c counts") {
    CHECK(grep({"-c", "a"}, "a\nb\nab\n").out == "2\n");
}
