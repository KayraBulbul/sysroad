#include "doctest.h"
#include "tally.hpp"

TEST_CASE("counts newlines") { CHECK(count_lines("a\nb\n") == 2); }
