#include "doctest.h"
#include "tally.hpp"

TEST_CASE("counts words split by spaces") { CHECK(count_words("a b c") == 3); }

TEST_CASE("a hyphen joins a word") { CHECK(count_words("well-known") == 1); }
