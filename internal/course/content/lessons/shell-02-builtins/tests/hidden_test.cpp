#include "doctest.h"

#include <unistd.h>

#include <climits>
#include <cstdlib>
#include <string>

#include "conch_run.hpp"

namespace shtest {

inline std::string cwd_now() {
    char dir[PATH_MAX];
    return getcwd(dir, sizeof dir) != nullptr ? dir : "";
}

TEST_CASE("cd - goes back and says where; cd sets PWD and OLDPWD") {
    std::string start = cwd_now();
    Ran r = conch("cd /\nprintenv PWD OLDPWD\ncd -\npwd\n");
    CHECK(r.out == "/\n" + start + "\n" + start + "\n" + start + "\n");
}

TEST_CASE("cd with too many arguments, or none and no HOME") {
    Ran many = conch("cd a b\npwd\n");
    CHECK(many.out == cwd_now() + "\n");
    CHECK(contains(many.err, "conch: cd:"));
    Ran home = conch("unset HOME\ncd\n");
    CHECK(home.status == 1);
    CHECK(contains(home.err, "conch: cd:"));
}

TEST_CASE("exit: no number, a bad number, too many, and only the low 8 bits") {
    CHECK(conch("false\nexit\necho never\n").status == 1);
    Ran bad = conch("exit lots\necho never\n");
    CHECK(bad.status == 2);
    CHECK(bad.out == "");
    CHECK(bad.err == "conch: exit: lots: numeric argument required\n");
    Ran many = conch("exit 1 2\necho still here\n");
    CHECK(many.out == "still here\n");
    CHECK(contains(many.err, "conch: exit:"));
    CHECK(conch("exit 258\n").status == 2);
}

TEST_CASE("export: values, a name alone, and bad names") {
    Ran r = conch("export SR_A=1 SR_B=x=y SR_EMPTY 1BAD=x\nprintenv SR_A SR_B SR_EMPTY\n");
    CHECK(r.out == "1\nx=y\n\n");
    CHECK(r.err == "conch: export: 1BAD=x: not a valid name\n");
    CHECK(conch("export 1BAD=x\n").status == 1);
    CHECK(conch("export A-B=x\n").status == 1);
    CHECK(conch("export =x\n").status == 1);
}

TEST_CASE("unset removes several at once") {
    Ran r = conch("export SR_X=1 SR_Y=2\nunset SR_X SR_Y\nprintenv SR_X\nprintenv SR_Y\necho [done]\n");
    CHECK(r.out == "[done]\n");
}

TEST_CASE("type: builtins, programs, names with a slash, and neither") {
    Ran r = conch("type pwd sh /bin/sh no-such-command-anywhere\n");
    CHECK(r.out.rfind("pwd is a shell builtin\nsh is /", 0) == 0);
    CHECK(contains(r.out, "/bin/sh is /bin/sh\n"));
    CHECK(r.err == "conch: type: no-such-command-anywhere: not found\n");
    CHECK(r.status == 1);
    write_file("not-exec.txt", "x\n");
    CHECK(conch("type ./not-exec.txt\n").status == 1);
}

TEST_CASE("every builtin is one") {
    Ran r = conch("type cd pwd exit export unset type\n");
    CHECK(r.out ==
          "cd is a shell builtin\npwd is a shell builtin\nexit is a shell builtin\n"
          "export is a shell builtin\nunset is a shell builtin\ntype is a shell builtin\n");
}

}  // namespace shtest
