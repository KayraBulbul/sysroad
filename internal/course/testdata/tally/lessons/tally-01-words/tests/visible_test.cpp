#include "doctest.h"
#include "sample.hpp"
#include "tally.hpp"

TEST_CASE("counts words split by spaces") { CHECK(count_words("a b c") == 3); }

TEST_CASE("a hyphen joins a word") { CHECK(count_words("well-known") == 1); }

TEST_CASE("counts the sample's words") { CHECK(count_words(sample()) == 3); }
