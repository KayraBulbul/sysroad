#include "doctest.h"

#include "bench.hpp"

struct FakeTime {
    double t = 0;
    Clock clock() {
        return [this] { return t; };
    }
};

TEST_CASE("summarize doesn't depend on the order") {
    Stats s = summarize({9, 2, 7, 4, 5, 1, 8});
    CHECK(s.min == 1);
    CHECK(s.median == 5);
    CHECK(s.max == 9);
    CHECK(s.samples == 7);
    CHECK(s.reps == 0);
    Stats one = summarize({42});
    CHECK(one.min == 42);
    CHECK(one.median == 42);
    CHECK(one.max == 42);
}

TEST_CASE("one slow call is enough when it reaches the target") {
    FakeTime ft;
    CHECK(calibrate([&] { ft.t += 5000; }, 1000, ft.clock()) == 1);
}

TEST_CASE("calibrate stops at max_reps when f takes no time") {
    FakeTime ft;
    long calls = 0;
    CHECK(calibrate([&] { ++calls; }, 1000, ft.clock()) == max_reps);
    CHECK(calls < 2 * max_reps);
}

TEST_CASE("calibrate times each batch on its own") {
    FakeTime ft;
    // Reading this clock moves it 5 µs, so every batch measures 5 µs more
    // than f took: 5001, 5002, 5004 and 5008 ns for 1, 2, 4 and 8 calls.
    // Timing from anywhere but the start of the batch gets more.
    Clock slow_to_read = [&] {
        double now = ft.t;
        ft.t += 5000;
        return now;
    };
    CHECK(calibrate([&] { ft.t += 1; }, 5008, slow_to_read) == 8);
}

TEST_CASE("measure doesn't count calibrate's calls") {
    FakeTime ft;
    long calls = 0;
    // The first call is slow, like a cold cache. Later ones take 10 ns.
    auto f = [&] { ft.t += ++calls == 1 ? 1000 : 10; };
    Stats s = measure(f, 3, 100, ft.clock());
    CHECK(s.max == 10);
}

TEST_CASE("the median ignores one slow sample") {
    FakeTime ft;
    long calls = 0;
    // reps settles at 4 (40 ns). Then one call during the samples takes 1 ms.
    auto f = [&] {
        ++calls;
        ft.t += calls == 10 ? 1e6 : 10;
    };
    Stats s = measure(f, 5, 40, ft.clock());
    CHECK(s.reps == 4);
    CHECK(s.median == 10);
    CHECK(s.min == 10);
    CHECK(s.max > 1000);
}
