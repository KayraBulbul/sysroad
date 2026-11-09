#pragma once
// Force-included into every test file: makes doctest show strings quoted and
// escaped, so "x " vs "x" (or a stray '\n') is visible in a failure, and
// vectors, arrays and pairs as their elements instead of {?}.
#include "doctest.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sysroad_detail {
inline doctest::String quoted(std::string_view s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\x%02x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += '"';
    return doctest::String(out.c_str(), static_cast<doctest::String::size_type>(out.size()));
}

// {1, 2, 3}, with at most 20 elements shown.
template <typename It> doctest::String list(It first, It last, std::size_t n) {
    doctest::String out = "{";
    std::size_t i = 0;
    for (; first != last && i < 20; ++first, ++i) {
        if (i > 0) out += ", ";
        out += doctest::toString(*first);
    }
    if (n > 20) {
        out += ", ... (";
        out += doctest::toString(n);
        out += " in all)";
    }
    return out + "}";
}
}  // namespace sysroad_detail

namespace doctest {
template <> struct StringMaker<std::string> {
    static String convert(const std::string& s) { return sysroad_detail::quoted(s); }
};
template <> struct StringMaker<std::string_view> {
    static String convert(std::string_view s) { return sysroad_detail::quoted(s); }
};
template <> struct StringMaker<const char*> {
    static String convert(const char* s) { return s ? sysroad_detail::quoted(s) : String("nullptr"); }
};
template <> struct StringMaker<char*> {
    static String convert(char* s) { return s ? sysroad_detail::quoted(s) : String("nullptr"); }
};
template <std::size_t N> struct StringMaker<char[N]> {
    static String convert(const char (&s)[N]) { return sysroad_detail::quoted(std::string_view(s, N ? N - 1 : 0)); }
};
template <std::size_t N> struct StringMaker<const char[N]> {
    static String convert(const char (&s)[N]) { return sysroad_detail::quoted(std::string_view(s, N ? N - 1 : 0)); }
};
template <typename T, typename A> struct StringMaker<std::vector<T, A>> {
    static String convert(const std::vector<T, A>& v) { return sysroad_detail::list(v.begin(), v.end(), v.size()); }
};
template <typename T, std::size_t N> struct StringMaker<std::array<T, N>> {
    static String convert(const std::array<T, N>& a) { return sysroad_detail::list(a.begin(), a.end(), N); }
};
template <typename A, typename B> struct StringMaker<std::pair<A, B>> {
    static String convert(const std::pair<A, B>& p) {
        return String("(") + toString(p.first) + ", " + toString(p.second) + ")";
    }
};
}  // namespace doctest
