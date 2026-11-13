#include "doctest.h"

#include <sys/resource.h>

#include <cstring>
#include <set>
#include <sstream>
#include <system_error>

#include "helpers.hpp"
#include "redirect.hpp"

static std::vector<Redirect> parse_all(std::initializer_list<const char*> words) {
    std::vector<Redirect> out;
    for (const char* w : words) {
        auto r = parse_redirect(w);
        REQUIRE_MESSAGE(r.has_value(), w);
        out.push_back(*r);
    }
    return out;
}

TEST_CASE("parse_redirect: every form") {
    auto r = parse_redirect("<&3");
    REQUIRE(r);
    CHECK(r->fd == 0);
    CHECK(r->op == Redirect::Dup);
    CHECK(r->from == 3);
    r = parse_redirect(">&2");
    REQUIRE(r);
    CHECK(r->fd == 1);
    CHECK(r->from == 2);
    r = parse_redirect(">>log.txt");
    REQUIRE(r);
    CHECK(r->fd == 1);
    CHECK(r->op == Redirect::Append);
    CHECK(r->path == "log.txt");
    r = parse_redirect("10<data");
    REQUIRE(r);
    CHECK(r->fd == 10);
    CHECK(r->op == Redirect::Read);
    CHECK(r->path == "data");
    r = parse_redirect("0>/dev/null");
    REQUIRE(r);
    CHECK(r->fd == 0);
    CHECK(r->op == Redirect::Write);
}

TEST_CASE("parse_redirect: things that aren't redirections") {
    for (const char* w : {"", "plain", "<", ">", ">>", "2>", "x>out", "2x>out", ">&", ">&x", "2>&1x",
                          "<<here", "a<b"}) {
        CHECK_MESSAGE(parse_redirect(w) == std::nullopt, w);
    }
}

TEST_CASE("redirections apply in order") {
    {
        CaptureStdout cap("orig.txt");
        Status s = run_redirected({"sh", "-c", "echo out; echo err >&2"},
                                  parse_all({"2>&1", ">only-out.txt"}));
        CHECK(s.ok());
    }
    CHECK(slurp("only-out.txt") == "out\n");
    CHECK(slurp("orig.txt") == "err\n");
}

TEST_CASE("> empties the file, >> appends") {
    write_file("t.txt", "a much longer line\n");
    run_redirected({"echo", "one"}, parse_all({">t.txt"}));
    CHECK(slurp("t.txt") == "one\n");
    run_redirected({"echo", "two"}, parse_all({">>t.txt"}));
    run_redirected({"echo", "three"}, parse_all({">>t.txt"}));
    CHECK(slurp("t.txt") == "one\ntwo\nthree\n");
    run_redirected({"echo", "new"}, parse_all({">>fresh.txt"}));
    CHECK(slurp("fresh.txt") == "new\n");
}

TEST_CASE("numbered fds, and copying them") {
    write_file("n.txt", "via three\n");
    Status s = run_redirected({"cat"}, parse_all({"3<n.txt", "<&3", ">n-out.txt"}));
    CHECK(s.ok());
    CHECK(slurp("n-out.txt") == "via three\n");
    s = run_redirected({"sh", "-c", "echo hi >&5"}, parse_all({"5>five.txt"}));
    CHECK(s.ok());
    CHECK(slurp("five.txt") == "hi\n");
}

TEST_CASE("a failed redirection: exit 1, and the command doesn't run") {
    Status s = run_redirected({"sh", "-c", "touch ran.txt"}, parse_all({"<missing.txt"}));
    CHECK(s.kind == Status::Exited);
    CHECK(s.code == 1);
    CHECK_FALSE(exists("ran.txt"));
    s = run_redirected({"sh", "-c", "touch ran.txt"}, parse_all({">no/such/dir/out.txt"}));
    CHECK(s.code == 1);
    s = run_redirected({"sh", "-c", "touch ran.txt"}, parse_all({"<&9"}));
    CHECK(s.code == 1);
    CHECK_FALSE(exists("ran.txt"));
}

TEST_CASE("an fd above the limit is an error, not ignored") {
    rlimit old{};
    getrlimit(RLIMIT_NOFILE, &old);
    rlimit low = old;
    low.rlim_cur = 64;
    setrlimit(RLIMIT_NOFILE, &low);
    Status s = run_redirected({"sh", "-c", "touch ran.txt"}, parse_all({"100>limit.txt"}));
    setrlimit(RLIMIT_NOFILE, &old);
    CHECK(s.code == 1);
    CHECK_FALSE(exists("ran.txt"));
}

TEST_CASE("apply_redirect's error names the file") {
    try {
        apply_redirect(*parse_redirect("<definitely-missing.txt"));
        FAIL("no exception");
    } catch (const std::system_error& e) {
        CHECK(std::string(e.what()).rfind("definitely-missing.txt", 0) == 0);
        CHECK(e.code().value() == ENOENT);
    }
}

TEST_CASE("a command that isn't found exits 127") {
    CHECK(run_redirected({"surely-not-a-command"}, {}).code == 127);
}

TEST_CASE("the child gets no extra fds, and the parent keeps its own") {
    write_file("x.txt", "x\n");
    std::set<int> before = inherited_fds();
    std::stringstream script;
    script << "open=; for f in";
    for (int fd = 3; fd < 64; ++fd) {
        script << ' ' << fd;
    }
    // test -e runs in the shell itself and opens nothing, so only the fds
    // the shell was given show up.
    script << "; do [ -e /dev/fd/$f ] && open=\"$open $f\"; done; echo $open > fds.txt";
    Status s = run_redirected({"sh", "-c", script.str()}, parse_all({"<x.txt", ">o.txt", "2>e.txt"}));
    CHECK(s.ok());
    std::set<int> extra;
    std::istringstream in(slurp("fds.txt"));
    for (int fd; in >> fd;) {
        if (!before.count(fd)) {
            extra.insert(fd);
        }
    }
    CHECK_MESSAGE(extra.empty(), "an fd you opened was left open in the child");
    CHECK(inherited_fds() == before);
}
