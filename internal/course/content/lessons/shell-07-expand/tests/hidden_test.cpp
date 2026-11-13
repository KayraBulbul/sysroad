#include "doctest.h"

#include <unistd.h>

#include <cstdlib>

#include "shell.hpp"

static std::vector<std::string> words(const std::string& line) {
    std::vector<std::string> out;
    for (const Token& t : tokenize(line)) {
        out.push_back(t.text);
    }
    return out;
}

using Words = std::vector<std::string>;

TEST_CASE("tokenize: quotes hide operators and #") {
    CHECK(words("a '|' \"&&\" \; '#' b#c") == Words{"a", "'|'", "\"&&\"", "\;", "'#'", "b#c"});
    CHECK(words("echo \"a\"'b'c|d") == Words{"echo", "\"a\"'b'c", "|", "d"});
    CHECK(words("echo \"say \\\"hi\\\"\"") == Words{"echo", "\"say \\\"hi\\\"\""});
}

TEST_CASE("tokenize: unclosed quotes") {
    CHECK_THROWS_AS(tokenize("echo 'abc"), SyntaxError);
    CHECK_THROWS_AS(tokenize("echo \"abc"), SyntaxError);
    CHECK_THROWS_AS(tokenize("echo \"abc\\\""), SyntaxError);
}

TEST_CASE("tokenize: what counts as an assignment") {
    CHECK(tokenize("A_1=x")[0].assignment);
    CHECK(tokenize("_=x")[0].assignment);
    CHECK(tokenize("A=")[0].assignment);
    CHECK_FALSE(tokenize("1A=x")[0].assignment);
    CHECK_FALSE(tokenize("=x")[0].assignment);
    CHECK_FALSE(tokenize("'A'=x")[0].assignment);
    CHECK_FALSE(tokenize("A-B=x")[0].assignment);
    CHECK(tokenize("A='x y'")[0].assignment);
}

TEST_CASE("expand_word: double quotes") {
    Shell sh;
    sh.vars["X"] = "v";
    CHECK(expand_word("\"\\$X \\\" \\\\ \\n\"", sh) == "$X \" \\ \\n");
    CHECK(expand_word("\"'$X'\"", sh) == "'v'");
    CHECK(expand_word("'\"$X\"'", sh) == "\"$X\"");
    CHECK(expand_word("\"$NOT_SET_ANYWHERE\"", sh) == "");
}

TEST_CASE("expand_word: dollars") {
    Shell sh;
    sh.vars["N"] = "n";
    sh.last_background = 4242;
    CHECK(expand_word("$$", sh) == std::to_string(getpid()));
    CHECK(expand_word("$!", sh) == "4242");
    CHECK(expand_word("a$", sh) == "a$");
    CHECK(expand_word("$", sh) == "$");
    CHECK(expand_word("$-x", sh) == "$-x");
    CHECK(expand_word("$N.$N", sh) == "n.n");
    CHECK(expand_word("$N_", sh) == std::nullopt);  // the name is N_
    CHECK_THROWS_AS(expand_word("${N", sh), SyntaxError);
}

TEST_CASE("expand_word: environment variables, and shell ones first") {
    Shell sh;
    setenv("SYSROAD_EXPAND", "env", 1);
    CHECK(expand_word("$SYSROAD_EXPAND", sh) == "env");
    sh.vars["SYSROAD_EXPAND_LOCAL"] = "local";
    CHECK(expand_word("$SYSROAD_EXPAND_LOCAL", sh) == "local");
}

TEST_CASE("expand_word: tilde") {
    Shell sh;
    setenv("HOME", "/h", 1);
    CHECK(expand_word("~", sh) == "/h");
    CHECK(expand_word("~/a", sh) == "/h/a");
    CHECK(expand_word("x~", sh) == "x~");
    CHECK(expand_word("'~'", sh) == "~");
    CHECK(expand_word("~user", sh) == "~user");
}

TEST_CASE("expand_word: a word that vanishes") {
    Shell sh;
    CHECK(expand_word("$NOT_SET_ANYWHERE$ALSO_NOT", sh) == std::nullopt);
    CHECK(expand_word("''$NOT_SET_ANYWHERE", sh) == "");
    CHECK(expand_word("\\$NOT_SET_ANYWHERE", sh) == "$NOT_SET_ANYWHERE");
}
