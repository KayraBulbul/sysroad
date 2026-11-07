#include "doctest.h"

#include <string>

#include "histogram.hpp"

TEST_CASE("add keeps adding up") {
    Histogram h;
    h.add("ab");
    h.add("bb");
    CHECK(h.count('b') == 3);
    CHECK(h.total() == 4);
}

TEST_CASE("a fresh histogram") {
    Histogram h;
    CHECK(h.total() == 0);
    CHECK(h.most_common() == 0);
}

TEST_CASE("ties go to the smallest byte") {
    Histogram h;
    h.add("zzaa");
    CHECK(h.most_common() == 'a');
}

TEST_CASE("bytes above 127") {
    Histogram h;
    h.add(std::string("\xff\xff\x80", 3));
    CHECK(h.count(0xff) == 2);
    CHECK(h.count(0x80) == 1);
    CHECK(h.most_common() == 0xff);
}

TEST_CASE("zero bytes count too") {
    Histogram h;
    h.add(std::string(10, '\0'));
    h.add("abc");
    CHECK(h.count(0) == 10);
    CHECK(h.most_common() == 0);
    CHECK(h.total() == 13);
}

TEST_CASE("const objects can be read") {
    Histogram h;
    h.add("xyz");
    const Histogram& view = h;
    CHECK(view.total() == 3);
    CHECK(view.count('y') == 1);
}
