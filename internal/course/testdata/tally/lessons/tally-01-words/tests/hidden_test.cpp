#include "doctest.h"
#include "tally.hpp"

TEST_CASE("tabs and newlines split words") { CHECK(count_words("a\tb\nc") == 3); }
