#include "doctest.h"

#include <vector>

#include "reader.hpp"

static const std::vector<unsigned char> data = {0x01, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 0xab};

TEST_CASE("reads numbers in order") {
    ByteReader r(data);
    CHECK(r.u8() == 0x01);
    CHECK(r.le16() == 0x1234);
    CHECK(r.le32() == 0x12345678);
    CHECK(r.pos() == 7);
    CHECK(r.remaining() == 1);
    CHECK(r.ok());
}

TEST_CASE("bytes returns a view into the buffer") {
    ByteReader r(data);
    r.skip(3);
    auto b = r.bytes(2);
    REQUIRE(b.size() == 2);
    CHECK(b.data() == data.data() + 3);
    CHECK(b[0] == 0x78);
}

TEST_CASE("reading past the end") {
    ByteReader r(data);
    r.skip(6);
    CHECK(r.le32() == 0);
    CHECK_FALSE(r.ok());
    CHECK(r.pos() == 6);  // a failed read doesn't move
}
