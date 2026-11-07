#pragma once
// Force-included into every test file: makes doctest show strings quoted and
// escaped, so "x " vs "x" (or a stray '\n') is visible in a failure.
#include "doctest.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>

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
}  // namespace doctest
