#include "doctest.h"

#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

#include "alloc_count.hpp"
#include "string.hpp"

static_assert(std::is_nothrow_move_constructible_v<String>);
static_assert(std::is_nothrow_move_assignable_v<String>);

TEST_CASE("empty strings") {
    String e;
    CHECK(e.size() == 0);
    CHECK(std::strcmp(e.c_str(), "") == 0);
    String f("");
    CHECK(f.empty());
    CHECK(f.is_inline());
}

TEST_CASE("append across the inline limit, and terminators everywhere") {
    String s("0123456789");
    s.append("abcde");
    CHECK(s.is_inline());
    CHECK(std::strcmp(s.c_str(), "0123456789abcde") == 0);
    s.append("f");
    CHECK_FALSE(s.is_inline());
    CHECK(std::strcmp(s.c_str(), "0123456789abcdef") == 0);
    s += "!";
    CHECK(s.size() == 17);
    CHECK(s.capacity() >= 17);
    CHECK(std::strcmp(s.c_str(), "0123456789abcdef!") == 0);
}

TEST_CASE("append(s) with itself") {
    String a("abc");
    a.append(a);
    CHECK(a == "abcabc");
    String b("0123456789");
    b.append(b);  // grows past 15 while reading from itself
    CHECK(b == "01234567890123456789");
    b.append(b);  // and again, on the heap
    CHECK(b == "0123456789012345678901234567890123456789");
    String c("xyz");
    c.append(std::string_view(c).substr(1));
    CHECK(c == "xyzyz");
}

TEST_CASE("copy assignment in every direction") {
    AllocScope scope;
    {
        String shortish("short");
        String longish("this one is much longer than fifteen");
        String x("x");
        x = longish;
        CHECK(x == longish);
        x = shortish;
        CHECK(x == "short");
        CHECK(std::strcmp(x.c_str(), "short") == 0);
        String& same = x;
        x = same;
        CHECK(x == "short");
        longish = longish;
        CHECK(longish == "this one is much longer than fifteen");
    }
    CHECK(scope.live() == 0);
}

TEST_CASE("move assignment in every direction") {
    AllocScope scope;
    {
        String a("a long string that lives on the heap");
        String b("short");
        String c("another long string that lives on the heap");
        b = std::move(a);  // heap into inline
        CHECK(b == "a long string that lives on the heap");
        CHECK(a.empty());
        c = std::move(b);  // heap into heap: c's old buffer must be freed
        CHECK(c == "a long string that lives on the heap");
        String d("tiny");
        c = std::move(d);  // inline into heap
        CHECK(c == "tiny");
        CHECK(d.empty());
        String& same = c;
        c = std::move(same);
        CHECK(c == "tiny");
        a = String("reused after a move");
        CHECK(a == "reused after a move");
    }
    CHECK(scope.live() == 0);
}

TEST_CASE("moving an inline string") {
    String a("inline");
    String b(std::move(a));
    CHECK(b == "inline");
    CHECK(b.is_inline());
    CHECK(a.empty());
    CHECK(std::strcmp(a.c_str(), "") == 0);
}

TEST_CASE("reserve and clear keep what they should") {
    String s("abc");
    s.reserve(100);
    CHECK(s.capacity() >= 100);
    CHECK(s == "abc");
    AllocScope scope;
    for (int i = 0; i < 90; ++i) {
        s.push_back('z');
    }
    CHECK(scope.news() == 0);
    s.clear();
    CHECK(s.empty());
    CHECK(s.capacity() >= 100);
    CHECK(std::strcmp(s.c_str(), "") == 0);
    s.reserve(5);  // never shrinks
    CHECK(s.capacity() >= 100);
}

TEST_CASE("bytes, comparison and hashing") {
    String a(std::string_view("a\0b", 3));
    CHECK(a.size() == 3);
    String b = a;
    CHECK(b == std::string_view("a\0b", 3));
    CHECK(String("apple") < String("banana"));
    CHECK(String("b") > String("a"));
    CHECK(std::hash<String>{}(String("key")) == std::hash<std::string_view>{}("key"));
}

TEST_CASE("nothing leaks") {
    AllocScope scope;
    {
        String s;
        for (int i = 0; i < 200; ++i) {
            s.append("xy");
            String t = s;
            String u = std::move(t);
            u.append(u);
        }
    }
    CHECK(scope.live() == 0);
}
