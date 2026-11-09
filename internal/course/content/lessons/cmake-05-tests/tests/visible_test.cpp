#include "doctest.h"

// The tests are a separate program that links textutil. They find this
// header only if textutil shares its include folder with whoever links it.
#include "textutil/textutil.hpp"

TEST_CASE("count counts like wc") {
    CHECK(textutil::count("one two\nthree\n") == textutil::Counts{2, 3, 14});
}

TEST_CASE("split_words and trim") {
    auto words = textutil::split_words("  to be\tor\nnot ");
    REQUIRE(words.size() == 4);
    CHECK(words[3] == "not");
    CHECK(textutil::trim("  hi there \n") == "hi there");
}
