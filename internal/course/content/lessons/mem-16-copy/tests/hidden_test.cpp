#include "doctest.h"

#include <string>
#include <vector>

#include "alloc_count.hpp"
#include "buffer.hpp"

TEST_CASE("self-assignment") {
    Buffer a(std::string_view("same"));
    Buffer& alias = a;
    a = alias;
    CHECK(a.view() == "same");
}

TEST_CASE("a copy allocates exactly once") {
    Buffer a(std::string_view("hello"));
    AllocScope s;
    Buffer b = a;
    CHECK(s.news() == 1);
    Buffer c(std::string_view("x"));
    AllocScope s2;
    c = a;
    CHECK(s2.news() == 1);
    CHECK(s2.deletes() == 1);
}

TEST_CASE("empty buffers") {
    AllocScope s;
    {
        Buffer e;
        Buffer f = e;
        Buffer g(std::string_view("x"));
        g = e;
        CHECK(g.size() == 0);
        CHECK(f.view() == "");
    }
    CHECK(s.live() == 0);
}

TEST_CASE("bytes with zeros in them") {
    Buffer a(std::string_view("a\0b", 3));
    Buffer b = a;
    CHECK(b.size() == 3);
    CHECK(b.view() == std::string_view("a\0b", 3));
}

TEST_CASE("chained assignment and a vector of copies") {
    AllocScope s;
    {
        Buffer a(std::string_view("one"));
        Buffer b(std::string_view("two"));
        Buffer c(std::string_view("three"));
        a = b = c;
        CHECK(a.view() == "three");
        CHECK(b.view() == "three");
        std::vector<Buffer> v(5, a);
        v[2][0] = 'T';
        CHECK(v[1].view() == "three");
        CHECK(v[2].view() == "Three");
        CHECK(a.view() == "three");
    }
    CHECK(s.live() == 0);
}
