#include "doctest.h"

#include "shell.hpp"

static std::vector<std::string> words(const std::string& line) {
    std::vector<std::string> out;
    for (const Token& t : tokenize(line)) {
        out.push_back(t.text);
    }
    return out;
}

using Words = std::vector<std::string>;

TEST_CASE("tokenize keeps quoted text together, as written") {
    CHECK(words("echo 'a b' \"c | d\" e\\ f") == Words{"echo", "'a b'", "\"c | d\"", "e\\ f"});
    CHECK(words("x=1 y") == Words{"x=1", "y"});
    CHECK(tokenize("x=1 y")[0].assignment);
    CHECK_FALSE(tokenize("x=1 y")[1].assignment);
}

TEST_CASE("expand_word") {
    Shell sh;
    sh.vars["X"] = "world";
    sh.last_status = 3;
    CHECK(expand_word("'$X'", sh) == "$X");
    CHECK(expand_word("\"hi $X\"", sh) == "hi world");
    CHECK(expand_word("${X}s", sh) == "worlds");
    CHECK(expand_word("$?", sh) == "3");
    CHECK(expand_word("a\\ b", sh) == "a b");
    CHECK(expand_word("$UNSET_ANYWHERE", sh) == std::nullopt);
    CHECK(expand_word("\"\"", sh) == "");
}
