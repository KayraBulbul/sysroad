// textutil's tests, as a plain program: each CHECK that fails is reported,
// and the exit status tells CTest whether the whole program passed.
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
