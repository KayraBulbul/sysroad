#include "doctest.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "alloc_count.hpp"
#include "counted.hpp"
#include "vector.hpp"

static_assert(std::is_nothrow_move_constructible_v<Vector<std::string>>);

TEST_CASE("growth moves, never copies") {
    Counted::reset();
    {
        Vector<Counted> v;
        for (int i = 0; i < 100; ++i) {
            v.emplace_back("x");
        }
        CHECK(Counted::copies == 0);
        CHECK(Counted::live == 100);
    }
    CHECK(Counted::live == 0);
}

TEST_CASE("push_back(v[0]) while growing") {
    Vector<std::string> v;
    v.push_back("a string long enough to be on the heap");
    for (int i = 0; i < 10; ++i) {
        v.push_back(v[0]);
        v.push_back(v.back());
    }
    CHECK(v.size() == 21);
    CHECK(v[20] == "a string long enough to be on the heap");
}

TEST_CASE("move-only elements") {
    Vector<std::unique_ptr<int>> v;
    for (int i = 0; i < 10; ++i) {
        v.push_back(std::make_unique<int>(i));
    }
    CHECK(*v[9] == 9);
    Vector<std::unique_ptr<int>> w = std::move(v);
    CHECK(v.size() == 0);
    CHECK(*w[3] == 3);
}

TEST_CASE("copies are deep and complete") {
    Counted::reset();
    {
        Vector<Counted> a;
        a.emplace_back("one");
        a.emplace_back("two");
        Vector<Counted> b = a;
        b[0].value = "changed";
        CHECK(a[0].value == "one");
        CHECK(Counted::live == 4);
        Vector<Counted> c;
        c.emplace_back("old");
        c = a;
        CHECK(c.size() == 2);
        CHECK(c[1].value == "two");
        Vector<Counted>& same = c;
        c = same;
        CHECK(c.size() == 2);
        CHECK(Counted::live == 6);
    }
    CHECK(Counted::live == 0);
}

TEST_CASE("moves steal and free what was there") {
    AllocScope s;
    {
        Vector<std::string> a = {"x", "y", "z"};
        Vector<std::string> b = {"old"};
        AllocScope m;
        b = std::move(a);
        CHECK(m.news() == 0);
        CHECK(b.size() == 3);
        CHECK(a.size() == 0);
        CHECK(a.capacity() == 0);
        a.push_back("reused");  // a moved-from Vector still works
        CHECK(a[0] == "reused");
        Vector<std::string>& same = b;
        b = std::move(same);
        CHECK(b.size() == 3);
    }
    CHECK(s.live() == 0);
}

TEST_CASE("clear, resize, pop_back") {
    Counted::reset();
    {
        Vector<Counted> v;
        v.resize(5);
        CHECK(v.size() == 5);
        CHECK(Counted::live == 5);
        CHECK(v[4].value == "default");
        v.resize(2);
        CHECK(Counted::live == 2);
        std::size_t cap = v.capacity();
        v.clear();
        CHECK(Counted::live == 0);
        CHECK(v.empty());
        CHECK(v.capacity() == cap);
        v.emplace_back("again");
        CHECK(v[0].value == "again");
    }
    CHECK(Counted::live == 0);
    Vector<int> ints;
    ints.resize(4);
    CHECK(ints[3] == 0);  // value-initialised
}

TEST_CASE("reserve never shrinks and keeps the elements") {
    Vector<std::string> v = {"a", "b"};
    v.reserve(50);
    CHECK(v.capacity() >= 50);
    v.reserve(1);
    CHECK(v.capacity() >= 50);
    CHECK(v[1] == "b");
}

TEST_CASE("an empty Vector allocates nothing") {
    AllocScope s;
    {
        Vector<std::string> v;
        Vector<std::string> w = v;
        Vector<std::string> x = std::move(w);
        x.clear();
    }
    CHECK(s.news() == 0);
}
