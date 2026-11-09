// The tests from last lesson, still on check.hpp. Rewrite them as doctest
// TEST_CASEs: one per function, with CHECK inside.
#include "check.hpp"
#include "textutil/textutil.hpp"

int main() {
    CHECK(textutil::count("a b\n") == (textutil::Counts{1, 2, 4}));
    CHECK(textutil::count("") == textutil::Counts{});

    auto words = textutil::split_words("  to be\tor  ");
    CHECK(words.size() == 3);
    CHECK(!words.empty() && words.front() == "to");

    CHECK(textutil::trim("  x y \n") == "x y");
    CHECK(textutil::trim(" \t ").empty());
    return check_result();
}
