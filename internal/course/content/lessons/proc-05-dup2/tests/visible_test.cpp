#include "doctest.h"

#include "helpers.hpp"
#include "redirect.hpp"

static Redirect file(int fd, Redirect::Op op, const char* path) {
    Redirect r;
    r.fd = fd;
    r.op = op;
    r.path = path;
    return r;
}

static Redirect dup(int fd, int from) {
    Redirect r;
    r.fd = fd;
    r.op = Redirect::Dup;
    r.from = from;
    return r;
}

TEST_CASE("parse_redirect") {
    CHECK(parse_redirect("<in.txt") == file(0, Redirect::Read, "in.txt"));
    CHECK(parse_redirect(">out.txt") == file(1, Redirect::Write, "out.txt"));
    CHECK(parse_redirect("2>>log") == file(2, Redirect::Append, "log"));
    CHECK(parse_redirect("2>&1") == dup(2, 1));
    CHECK(parse_redirect("hello") == std::nullopt);
}

TEST_CASE("stdin and stdout from and to files") {
    write_file("in.txt", "quiet\n");
    Status s = run_redirected({"tr", "a-z", "A-Z"}, {*parse_redirect("<in.txt"),
                                                     *parse_redirect(">out.txt")});
    CHECK(s.ok());
    CHECK(slurp("out.txt") == "QUIET\n");
}

TEST_CASE(">file 2>&1 sends both to the file") {
    Status s = run_redirected({"sh", "-c", "echo out; echo err >&2"},
                              {*parse_redirect(">both.txt"), *parse_redirect("2>&1")});
    CHECK(s.ok());
    CHECK(slurp("both.txt") == "out\nerr\n");
}
