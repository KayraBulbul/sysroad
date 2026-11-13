#include "doctest.h"

#include "capture.hpp"

TEST_CASE("capture what a command prints") {
    Output o = capture({"echo", "hello", "pipes"});
    CHECK(o.out == "hello pipes\n");
    CHECK(o.status.ok());
}

TEST_CASE("capture keeps the exit status") {
    Output o = capture({"sh", "-c", "echo partial; exit 3"});
    CHECK(o.out == "partial\n");
    CHECK(o.status.code == 3);
}

TEST_CASE("communicate: input in, output out") {
    Output o = communicate({"tr", "a-z", "A-Z"}, "shout\n");
    CHECK(o.out == "SHOUT\n");
    CHECK(o.status.ok());
}
