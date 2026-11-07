#include "doctest.h"

#include <cstddef>
#include <string>
#include <string_view>

std::size_t count_words(std::string_view text);

TEST_CASE("count_words: only whitespace") {
    CHECK(count_words("   ") == 0);
    CHECK(count_words("\v\f\r\n\t ") == 0);
}

TEST_CASE("count_words: tabs and newlines separate words") {
    CHECK(count_words("\t\ttabs\tand\nnewlines\n") == 3);
}

TEST_CASE("count_words: punctuation is part of a word") {
    CHECK(count_words("a,b c") == 2);
    CHECK(count_words("-- !") == 2);
}

TEST_CASE("count_words: UTF-8 bytes are word characters") {
    CHECK(count_words("h\xc3\xa9llo w\xc3\xb6rld") == 2);
    CHECK(count_words("\xff\xfe word") == 2);
}

TEST_CASE("count_words: a view into the middle of a buffer") {
    std::string_view all = "xx hello yy";
    CHECK(count_words(all.substr(3, 5)) == 1);
}

TEST_CASE("count_words: long input") {
    std::string s;
    for (int i = 0; i < 10000; ++i) {
        s += "ab ";
    }
    CHECK(count_words(s) == 10000);
}
