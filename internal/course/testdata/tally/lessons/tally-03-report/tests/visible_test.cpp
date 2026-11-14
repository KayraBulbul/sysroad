#include "doctest.h"
#include "tally.hpp"

TEST_CASE("a hyphen splits words now") { CHECK(count_words("well-known") == 2); }

TEST_CASE("the report lists words and lines") { CHECK(report("a b\n") == "words: 2\nlines: 1\n"); }
