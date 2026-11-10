#include "doctest.h"

#include <cstdint>
#include <numeric>

#include "mystery.hpp"

static std::uint64_t next(std::uint64_t& state) {
    state = state * 6364136223846793005u + 1442695040888963407u;
    return state >> 33;
}

TEST_CASE("alpha returns the larger argument") {
    std::uint64_t state = 1;
    for (int i = 0; i < 2000; ++i) {
        long a = static_cast<long>(next(state) % 2001) - 1000;
        long b = static_cast<long>(next(state) % 2001) - 1000;
        CAPTURE(a);
        CAPTURE(b);
        REQUIRE(alpha(a, b) == (a > b ? a : b));
    }
    CHECK(alpha(7, 7) == 7);
    CHECK(alpha(-1, 1) == 1);
    CHECK(alpha(1, -1) == 1);
}

TEST_CASE("bravo for every score around its limits") {
    for (int x = -20; x <= 120; ++x) {
        int expected = x >= 90 ? 4 : x >= 75 ? 3 : x >= 50 ? 2 : 0;
        CAPTURE(x);
        REQUIRE(bravo(x) == expected);
    }
    CHECK(bravo(-2000000000) == 0);
    CHECK(bravo(2000000000) == 4);
}

TEST_CASE("charlie multiplies n times") {
    for (long a = -9; a <= 9; ++a) {
        long expected = 1;
        for (unsigned long n = 0; n <= 15; ++n) {
            CAPTURE(a);
            CAPTURE(n);
            REQUIRE(charlie(a, n) == expected);
            expected *= a;
        }
    }
}

static long reference_delta(long n) {
    long steps = 0;
    while (n != 1) {
        n = n % 2 == 0 ? n / 2 : 3 * n + 1;
        ++steps;
    }
    return steps;
}

TEST_CASE("delta for the first few thousand numbers") {
    for (long n = 1; n <= 3000; ++n) {
        CAPTURE(n);
        REQUIRE(delta(n) == reference_delta(n));
    }
    CHECK(delta(27) == 111);
    CHECK(delta(1024) == 10);
}

TEST_CASE("echo") {
    std::uint64_t state = 5;
    for (int i = 0; i < 2000; ++i) {
        unsigned long a = next(state) % 100000;
        unsigned long b = next(state) % 100000;
        CAPTURE(a);
        CAPTURE(b);
        REQUIRE(echo(a, b) == std::gcd(a, b));
    }
    CHECK(echo(0, 0) == 0);
    CHECK(echo(0, 9) == 9);
    CHECK(echo(9, 0) == 9);
    CHECK(echo(17, 17) == 17);
    // Large enough that a signed type would get it wrong.
    CHECK(echo(18446744073709551615u, 4294967295u) == 4294967295u);
    CHECK(echo(9223372036854775808u, 6) == 2);
}
