#include "doctest.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>

#include "conch_pty.hpp"
#include "conch_run.hpp"

namespace shtest {

// A fresh home folder for one test, as $HOME.
inline std::string fresh_home(const std::string& name) {
    std::string home = test_dir() + "/" + name;
    mkdir(home.c_str(), 0755);
    setenv("HOME", home.c_str(), 1);
    return home;
}

TEST_CASE("on a terminal, history comes from ~/.conch_history and goes back there, readable only by you") {
    std::string home = fresh_home("home-visible");
    write_file(home + "/.conch_history", "echo from last time\n");
    Conch c;
    REQUIRE(c.prompt());
    c.line("history");
    CHECK_MESSAGE(c.expect("    1  echo from last time"), c.output());
    REQUIRE(c.prompt());
    c.line("echo new");
    REQUIRE(c.prompt());
    c.line("exit");
    REQUIRE(c.wait() == 0);
    CHECK(read_file(home + "/.conch_history") == "echo from last time\nhistory\necho new\nexit\n");
    struct stat st{};
    REQUIRE(stat((home + "/.conch_history").c_str(), &st) == 0);
    CHECK_MESSAGE((st.st_mode & 0777) == 0600, "the history file should be readable by its owner only (0600)");
}

TEST_CASE("history numbers the lines from 1") {
    Ran r = conch("echo a\necho b\nhistory\n");
    CHECK(r.out == "a\nb\n    1  echo a\n    2  echo b\n    3  history\n");
}

}  // namespace shtest
