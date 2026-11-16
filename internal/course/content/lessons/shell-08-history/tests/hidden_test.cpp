#include "doctest.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>

#include "conch_pty.hpp"
#include "conch_run.hpp"

namespace shtest {

inline std::string home_dir(const std::string& name) {
    std::string home = test_dir() + "/" + name;
    mkdir(home.c_str(), 0755);
    return home;
}

TEST_CASE("the history keeps the newest 1000 lines") {
    std::string script;
    for (int i = 1; i <= 1100; ++i) {
        script += "# " + std::to_string(i) + "\n";  // comments: they run nothing, but count
    }
    script += "history 2\n";
    Ran r = conch(script);
    CHECK(r.out == "  999  # 1100\n 1000  history 2\n");
}

TEST_CASE("a script doesn't touch ~/.conch_history") {
    std::string home = home_dir("home-script");
    Ran r = conch("echo hi\n", [home] { setenv("HOME", home.c_str(), 1); });
    CHECK(r.out == "hi\n");
    CHECK(access((home + "/.conch_history").c_str(), F_OK) == -1);
}

TEST_CASE("an old history file with looser permissions ends up 0600") {
    std::string home = home_dir("home-loose");
    setenv("HOME", home.c_str(), 1);
    write_file(home + "/.conch_history", "echo old\n");
    chmod((home + "/.conch_history").c_str(), 0644);
    Conch c;
    REQUIRE(c.prompt());
    c.line("exit");
    REQUIRE(c.wait() == 0);
    struct stat st{};
    REQUIRE(stat((home + "/.conch_history").c_str(), &st) == 0);
    CHECK((st.st_mode & 0777) == 0600);
    CHECK(read_file(home + "/.conch_history") == "echo old\nexit\n");
}

TEST_CASE("no history file yet: an empty history, and one is made") {
    std::string home = home_dir("home-new");
    setenv("HOME", home.c_str(), 1);
    Conch c;
    REQUIRE(c.prompt());
    c.line("history");
    CHECK_MESSAGE(c.expect("    1  history"), c.output());
    REQUIRE(c.prompt());
    c.send("\x04");  // Ctrl-D ends the input: the history is saved then too
    REQUIRE(c.wait() == 0);
    CHECK(read_file(home + "/.conch_history") == "history\n");
}

TEST_CASE("blank lines, repeats and a bad count") {
    Ran r = conch("true\n\ntrue\n   \nhistory\nhistory x\n");
    CHECK(r.out == "    1  true\n    2  history\n");
    CHECK(r.err == "conch: history: x: numeric argument required\n");
}

}  // namespace shtest
