#include "doctest.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "bitset.hpp"

static const std::size_t kSizes[] = {0, 1, 2, 63, 64, 65, 127, 128, 129, 200, 1000};

// Check a Bitset against a plain vector<bool> holding the same bits.
static void same(const Bitset& b, const std::vector<bool>& model) {
    REQUIRE(b.size() == model.size());
    std::size_t ones = 0;
    std::string text;
    for (std::size_t i = 0; i < model.size(); ++i) {
        CAPTURE(i);
        REQUIRE(b.test(i) == model[i]);
        ones += model[i];
        text += model[i] ? '1' : '0';
    }
    REQUIRE(b.count() == ones);
    REQUIRE(b.any() == (ones != 0));
    REQUIRE(b.to_string() == text);
    // Walk the set bits with find_first / find_next.
    std::size_t at = b.find_first();
    for (std::size_t i = 0; i < model.size(); ++i) {
        if (model[i]) {
            REQUIRE(at == i);
            at = b.find_next(at);
        }
    }
    REQUIRE(at == model.size());
}

static std::uint32_t next(std::uint32_t& state) {
    state = state * 1664525 + 1013904223;
    return state >> 8;
}

TEST_CASE("random sets, flips and clears match a vector<bool>") {
    std::uint32_t state = 42;
    for (std::size_t size : kSizes) {
        CAPTURE(size);
        Bitset b(size);
        std::vector<bool> model(size, false);
        same(b, model);
        if (size == 0) {
            continue;
        }
        for (int step = 0; step < 400; ++step) {
            std::size_t i = next(state) % size;
            switch (next(state) % 4) {
            case 0:
                b.set(i);
                model[i] = true;
                break;
            case 1:
                b.set(i, false);
                model[i] = false;
                break;
            case 2:
                b.set(i, true);
                model[i] = true;
                break;
            default:
                b.flip(i);
                model[i] = !model[i];
                break;
            }
        }
        same(b, model);
    }
}

TEST_CASE("the last bit of every size") {
    for (std::size_t size : kSizes) {
        if (size == 0) {
            continue;
        }
        CAPTURE(size);
        Bitset b(size);
        b.set(size - 1);
        CHECK(b.test(size - 1));
        CHECK(b.count() == 1);
        CHECK(b.find_first() == size - 1);
        CHECK(b.find_next(size - 1) == size);
        CHECK_THROWS_AS(b.test(size), std::out_of_range);
        CHECK_THROWS_AS(b.set(size), std::out_of_range);
        CHECK_THROWS_AS(b.set(size, false), std::out_of_range);
        CHECK_THROWS_AS(b.flip(size), std::out_of_range);
        CHECK_THROWS_AS(b.test(static_cast<std::size_t>(-1)), std::out_of_range);
    }
}

TEST_CASE("an empty bitset") {
    Bitset b(0);
    CHECK(b.size() == 0);
    CHECK(b.count() == 0);
    CHECK_FALSE(b.any());
    CHECK(b.find_first() == 0);
    CHECK(b.find_next(0) == 0);
    CHECK(b.to_string().empty());
    CHECK_THROWS_AS(b.test(0), std::out_of_range);
    b.set_all();
    CHECK(b.count() == 0);
    CHECK((~b).count() == 0);
    CHECK(~b == b);
}

TEST_CASE("set_all and ~ leave the unused bits alone") {
    for (std::size_t size : kSizes) {
        CAPTURE(size);
        Bitset all(size);
        all.set_all();
        CHECK(all.count() == size);
        CHECK(all.any() == (size != 0));

        Bitset none(size);
        Bitset flipped = ~none;
        CHECK(flipped.count() == size);
        CHECK(flipped == all);
        CHECK(~flipped == none);
        CHECK_FALSE((~flipped).any());
        CHECK((~flipped).find_first() == size);

        // Built bit by bit, it has to equal the one built by set_all.
        Bitset by_hand(size);
        for (std::size_t i = 0; i < size; ++i) {
            by_hand.set(i);
        }
        CHECK(by_hand == all);
    }
}

TEST_CASE("~ flips every bit and doesn't change the original") {
    Bitset b(130);
    b.set(0);
    b.set(64);
    b.set(129);
    Bitset inverse = ~b;
    CHECK(b.count() == 3);
    CHECK(inverse.count() == 127);
    CHECK_FALSE(inverse.test(0));
    CHECK_FALSE(inverse.test(64));
    CHECK_FALSE(inverse.test(129));
    CHECK(inverse.test(1));
    CHECK(inverse.test(128));
    CHECK(inverse.find_first() == 1);
}

TEST_CASE("find_next with any starting point") {
    Bitset b(300);
    for (std::size_t i : {0u, 63u, 64u, 65u, 191u, 192u, 299u}) {
        b.set(i);
    }
    CHECK(b.find_first() == 0);
    CHECK(b.find_next(0) == 63);
    CHECK(b.find_next(1) == 63);
    CHECK(b.find_next(62) == 63);
    CHECK(b.find_next(63) == 64);
    CHECK(b.find_next(64) == 65);
    CHECK(b.find_next(65) == 191);
    CHECK(b.find_next(100) == 191);
    CHECK(b.find_next(190) == 191);
    CHECK(b.find_next(191) == 192);
    CHECK(b.find_next(192) == 299);
    CHECK(b.find_next(298) == 299);
    CHECK(b.find_next(299) == 300);
    CHECK(b.find_next(300) == 300);
    CHECK(b.find_next(5000) == 300);
    CHECK(b.find_next(static_cast<std::size_t>(-1)) == 300);
}

TEST_CASE("the operators match bit-by-bit logic") {
    std::uint32_t state = 7;
    for (std::size_t size : kSizes) {
        CAPTURE(size);
        Bitset a(size);
        Bitset b(size);
        std::vector<bool> ma(size);
        std::vector<bool> mb(size);
        for (std::size_t i = 0; i < size; ++i) {
            ma[i] = next(state) % 2;
            mb[i] = next(state) % 3 == 0;
            a.set(i, ma[i]);
            b.set(i, mb[i]);
        }
        std::vector<bool> expect_and(size);
        std::vector<bool> expect_or(size);
        std::vector<bool> expect_xor(size);
        std::vector<bool> expect_not(size);
        for (std::size_t i = 0; i < size; ++i) {
            expect_and[i] = ma[i] && mb[i];
            expect_or[i] = ma[i] || mb[i];
            expect_xor[i] = ma[i] != mb[i];
            expect_not[i] = !ma[i];
        }
        Bitset r = a;
        same(r &= b, expect_and);
        r = a;
        same(r |= b, expect_or);
        r = a;
        same(r ^= b, expect_xor);
        same(~a, expect_not);
        same(a, ma);  // the operand wasn't changed
        same(b, mb);
    }
}

TEST_CASE("the operators return the left-hand side, so they chain") {
    Bitset a(100);
    Bitset b(100);
    Bitset c(100);
    a.set(1);
    b.set(2);
    c.set(99);
    Bitset& ref = (a |= b);
    CHECK(&ref == &a);
    (a |= b) |= c;
    CHECK(a.count() == 3);
    CHECK(a.test(99));
}

TEST_CASE("different sizes can't be combined") {
    Bitset a(64);
    Bitset b(65);
    CHECK_THROWS_AS(a &= b, std::invalid_argument);
    CHECK_THROWS_AS(a |= b, std::invalid_argument);
    CHECK_THROWS_AS(b ^= a, std::invalid_argument);
    CHECK_FALSE(a == b);
    CHECK(a == Bitset(64));
}

TEST_CASE("a million bits") {
    Bitset b(1000000);
    for (std::size_t i = 0; i < b.size(); i += 1000) {
        b.set(i);
    }
    CHECK(b.count() == 1000);
    // Searching has to skip the empty words, not test a million bits a thousand times.
    std::size_t found = 0;
    for (std::size_t i = b.find_first(); i < b.size(); i = b.find_next(i)) {
        ++found;
    }
    CHECK(found == 1000);
    Bitset inverse = ~b;
    CHECK(inverse.count() == 999000);
    inverse |= b;
    CHECK(inverse.count() == 1000000);
}
