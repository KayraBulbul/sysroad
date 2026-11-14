#include "doctest.h"
#include "sample.hpp"
#include "tally.hpp"

TEST_CASE("counts newlines") { CHECK(count_lines("a\nb\n") == 2); }

TEST_CASE("counts the sample's lines") { CHECK(count_lines(sample()) == 1); }
