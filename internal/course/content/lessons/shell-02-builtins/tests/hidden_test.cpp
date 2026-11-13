#include "doctest.h"

#include <fcntl.h>
#include <unistd.h>

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "shell.hpp"

// Where the tests started: cd moves us, and the files stay here.
static const std::string kHere = [] {
    char dir[PATH_MAX];
    return std::string(getcwd(dir, sizeof dir));
}();

// Runs a builtin with stdout and stderr in files; returns its status.
static int quietly(Shell& sh, const std::vector<std::string>& argv, std::string* out = nullptr) {
    std::fflush(nullptr);
    int saved1 = dup(1), saved2 = dup(2);
    int o = open((kHere + "/bi-out.txt").c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int e = open((kHere + "/bi-err.txt").c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(o, 1);
    dup2(e, 2);
    close(o);
    close(e);
    int status = find_builtin(argv[0])(sh, argv);
    std::fflush(nullptr);
    dup2(saved1, 1);
    dup2(saved2, 2);
    close(saved1);
    close(saved2);
    if (out != nullptr) {
        out->clear();
        FILE* f = std::fopen((kHere + "/bi-out.txt").c_str(), "r");
        for (int c; (c = std::fgetc(f)) != EOF;) {
            *out += static_cast<char>(c);
        }
        std::fclose(f);
    }
    return status;
}

struct BackToStart {
    char dir[PATH_MAX];
    BackToStart() {
        getcwd(dir, sizeof dir);
    }
    ~BackToStart() {
        chdir(dir);
    }
};

TEST_CASE("cd: OLDPWD, PWD, and cd -") {
    BackToStart back;
    Shell sh;
    REQUIRE(quietly(sh, {"cd", "/"}) == 0);
    CHECK(std::string(std::getenv("PWD")) == "/");
    CHECK(std::string(std::getenv("OLDPWD")) == back.dir);
    std::string out;
    CHECK(quietly(sh, {"cd", "-"}, &out) == 0);
    CHECK(out == std::string(back.dir) + "\n");
    char now[PATH_MAX];
    getcwd(now, sizeof now);
    CHECK(std::string(now) == back.dir);
}

TEST_CASE("cd: errors leave the folder alone") {
    BackToStart back;
    Shell sh;
    CHECK(quietly(sh, {"cd", "/no/such/folder"}) == 1);
    CHECK(quietly(sh, {"cd", "a", "b"}) == 1);
    char now[PATH_MAX];
    getcwd(now, sizeof now);
    CHECK(std::string(now) == back.dir);
}

TEST_CASE("pwd") {
    BackToStart back;
    Shell sh;
    std::string out;
    CHECK(quietly(sh, {"pwd"}, &out) == 0);
    CHECK(out == std::string(back.dir) + "\n");
}

TEST_CASE("exit: no number, a bad number, too many") {
    Shell sh;
    sh.last_status = 4;
    quietly(sh, {"exit"});
    CHECK(sh.exiting);
    CHECK(sh.exit_code == 4);
    Shell bad;
    quietly(bad, {"exit", "abc"});
    CHECK(bad.exiting);
    CHECK(bad.exit_code == 2);
    Shell many;
    CHECK(quietly(many, {"exit", "1", "2"}) == 1);
    CHECK_FALSE(many.exiting);
    Shell big;
    quietly(big, {"exit", "258"});
    CHECK(big.exit_code == 2);
}

TEST_CASE("export and unset") {
    Shell sh;
    CHECK(quietly(sh, {"export", "SYSROAD_A=1", "SYSROAD_B=x=y"}) == 0);
    CHECK(std::string(std::getenv("SYSROAD_A")) == "1");
    CHECK(std::string(std::getenv("SYSROAD_B")) == "x=y");
    CHECK(quietly(sh, {"export", "SYSROAD_EMPTY"}) == 0);
    REQUIRE(std::getenv("SYSROAD_EMPTY") != nullptr);
    CHECK(std::string(std::getenv("SYSROAD_EMPTY")).empty());
    CHECK(quietly(sh, {"export", "1BAD=x"}) == 1);
    CHECK(std::getenv("1BAD") == nullptr);
    CHECK(quietly(sh, {"unset", "SYSROAD_A", "SYSROAD_B"}) == 0);
    CHECK(std::getenv("SYSROAD_A") == nullptr);
    CHECK(std::getenv("SYSROAD_B") == nullptr);
}

TEST_CASE("type: builtins, programs, and neither") {
    Shell sh;
    std::string out;
    CHECK(quietly(sh, {"type", "pwd", "sh"}, &out) == 0);
    CHECK(out.rfind("pwd is a shell builtin\nsh is /", 0) == 0);
    CHECK(quietly(sh, {"type", "sh", "no-such-command-anywhere"}, &out) == 1);
}
