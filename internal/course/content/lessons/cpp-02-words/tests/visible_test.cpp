#include "doctest.h"

#include <cstddef>
#include <string_view>

std::size_t count_words(std::string_view text);

TEST_CASE("count_words: two words") {
    CHECK(count_words("hello world") == 2);
}

TEST_CASE("count_words: empty text") {
    CHECK(count_words("") == 0);
}

TEST_CASE("count_words: leading and trailing spaces") {
    CHECK(count_words("  leading and trailing  ") == 3);
}
