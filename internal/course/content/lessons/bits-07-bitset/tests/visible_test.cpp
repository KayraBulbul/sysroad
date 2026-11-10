#include "doctest.h"

#include <stdexcept>

#include "bitset.hpp"

TEST_CASE("a new bitset is all zeros") {
    Bitset b(10);
    CHECK(b.size() == 10);
    CHECK(b.count() == 0);
    CHECK_FALSE(b.any());
    CHECK(b.to_string() == "0000000000");
}

TEST_CASE("set, test and flip one bit") {
    Bitset b(10);
    b.set(3);
    b.set(7);
    CHECK(b.test(3));
    CHECK_FALSE(b.test(4));
    CHECK(b.count() == 2);
    CHECK(b.to_string() == "0001000100");
    b.flip(3);
    b.flip(0);
    b.set(7, false);
    CHECK(b.to_string() == "1000000000");
}

TEST_CASE("a bad index throws") {
    Bitset b(10);
    CHECK_THROWS_AS(b.test(10), std::out_of_range);
    CHECK_THROWS_AS(b.set(10), std::out_of_range);
    CHECK_THROWS_AS(b.flip(99), std::out_of_range);
}

TEST_CASE("bits beyond the first word") {
    Bitset b(200);
    b.set(64);
    b.set(199);
    CHECK(b.test(64));
    CHECK(b.test(199));
    CHECK_FALSE(b.test(0));
    CHECK(b.count() == 2);
}

TEST_CASE("set_all sets exactly size() bits") {
    Bitset b(70);
    b.set_all();
    CHECK(b.count() == 70);
    CHECK(b.any());
}

TEST_CASE("find_first and find_next") {
    Bitset b(200);
    CHECK(b.find_first() == 200);
    b.set(5);
    b.set(130);
    CHECK(b.find_first() == 5);
    CHECK(b.find_next(5) == 130);
    CHECK(b.find_next(130) == 200);
}

TEST_CASE("the operators") {
    Bitset a(8);
    Bitset b(8);
    a.set(0);
    a.set(1);
    b.set(1);
    b.set(2);
    Bitset both = a;
    both &= b;
    CHECK(both.to_string() == "01000000");
    Bitset either = a;
    either |= b;
    CHECK(either.to_string() == "11100000");
    Bitset differ = a;
    differ ^= b;
    CHECK(differ.to_string() == "10100000");
    CHECK((~a).to_string() == "00111111");
}
