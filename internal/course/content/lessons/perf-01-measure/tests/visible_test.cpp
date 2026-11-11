#include "doctest.h"

#include "bench.hpp"

// A fake clock: f moves it forward, now() reads it.
struct FakeTime {
    double t = 0;
    Clock clock() {
        return [this] { return t; };
    }
};

TEST_CASE("summarize") {
    Stats s = summarize({30, 10, 20});
    CHECK(s.min == 10);
    CHECK(s.median == 20);
    CHECK(s.max == 30);
    CHECK(s.samples == 3);
    CHECK(summarize({4, 1, 3, 2}).median == 2.5);
}

TEST_CASE("calibrate doubles reps until a batch takes target_ns") {
    FakeTime ft;
    auto f = [&] { ft.t += 100; };  // every call takes 100 ns
    // 1, 2, 4 and 8 calls take 100, 200, 400 and 800 ns. 16 take 1600.
    CHECK(calibrate(f, 1000, ft.clock()) == 16);
    CHECK(calibrate(f, 800, ft.clock()) == 8);
}

TEST_CASE("measure reports the time per call") {
    FakeTime ft;
    auto f = [&] { ft.t += 50; };
    Stats s = measure(f, 5, 1000, ft.clock());
    CHECK(s.reps == 32);
    CHECK(s.samples == 5);
    CHECK(s.min == 50);
    CHECK(s.median == 50);
    CHECK(s.max == 50);
}
