#include "doctest.h"

#include "alloc_count.hpp"
#include "buffer.hpp"

TEST_CASE("the destructor frees") {
    AllocScope s;
    {
        Buffer b(std::string_view("abc"));
    }
    CHECK(s.live() == 0);
}

TEST_CASE("a copy has its own bytes") {
    Buffer a(std::string_view("hello"));
    Buffer b = a;
    b[0] = 'j';
    CHECK(a.view() == "hello");
    CHECK(b.view() == "jello");
    CHECK(a.data() != b.data());
}

TEST_CASE("assignment copies, and frees the old bytes") {
    AllocScope s;
    {
        Buffer a(std::string_view("short"));
        Buffer c(std::string_view("a longer buffer"));
        c = a;
        CHECK(c.view() == "short");
        c[0] = 'S';
        CHECK(a.view() == "short");
    }
    CHECK(s.live() == 0);
}
