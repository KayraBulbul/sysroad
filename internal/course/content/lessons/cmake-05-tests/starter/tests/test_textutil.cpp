// textutil's tests, as a plain program: each CHECK that fails is reported,
// and the exit status tells CTest whether the whole program passed.
#include "check.hpp"
#include "textutil/textutil.hpp"

int main() {
    CHECK(textutil::count("a b\n") == (textutil::Counts{1, 2, 4}));
    // TODO: a few checks of split_words and trim.
    return check_result();
}
