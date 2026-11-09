#include "doctest.h"

#include <cstdint>
#include <span>
#include <vector>

#include "reader.hpp"

static const std::vector<unsigned char> data = {0xde, 0xad, 0xbe, 0xef, 0x00, 0x2a};

TEST_CASE("big-endian reads") {
    ByteReader r(data);
    CHECK(r.be32() == 0xdeadbeef);
    CHECK(r.be16() == 0x002a);
    CHECK(r.remaining() == 0);
    CHECK(r.ok());
}

TEST_CASE("reading exactly to the end is fine; one more byte isn't") {
    ByteReader r(data);
    CHECK(r.bytes(6).size() == 6);
    CHECK(r.ok());
    CHECK(r.bytes(0).empty());
    CHECK(r.ok());
    CHECK(r.u8() == 0);
    CHECK_FALSE(r.ok());
}

TEST_CASE("ok() stays false after a failure") {
    ByteReader r(data);
    r.skip(100);
    CHECK_FALSE(r.ok());
    CHECK(r.pos() == 0);
    CHECK(r.u8() == 0xde);  // later reads still work...
    CHECK_FALSE(r.ok());    // ...but the failure is remembered
}

TEST_CASE("a huge length can't wrap around") {
    ByteReader r(data);
    r.skip(2);
    CHECK(r.bytes(SIZE_MAX).empty());
    CHECK_FALSE(r.ok());
    CHECK(r.pos() == 2);
    ByteReader r2(data);
    r2.skip(SIZE_MAX - 1);
    CHECK_FALSE(r2.ok());
}

TEST_CASE("seek") {
    ByteReader r(data);
    r.seek(4);
    CHECK(r.be16() == 0x002a);
    r.seek(1);
    CHECK(r.u8() == 0xad);
    r.seek(6);
    CHECK(r.ok());
    CHECK(r.remaining() == 0);
    r.seek(7);
    CHECK_FALSE(r.ok());
    CHECK(r.pos() == 6);
}

TEST_CASE("each half-read number reads nothing") {
    for (int size = 0; size < 4; ++size) {
        ByteReader r(std::span(data).first(size));
        CHECK(r.le32() == 0);
        CHECK(r.be32() == 0);
        CHECK_FALSE(r.ok());
        CHECK(r.pos() == 0);
    }
    ByteReader r(std::span(data).first(1));
    CHECK(r.le16() == 0);
    CHECK(r.be16() == 0);
    CHECK_FALSE(r.ok());
}

TEST_CASE("an empty buffer") {
    ByteReader r(std::span<const unsigned char>{});
    CHECK(r.remaining() == 0);
    CHECK(r.u8() == 0);
    CHECK_FALSE(r.ok());
}
