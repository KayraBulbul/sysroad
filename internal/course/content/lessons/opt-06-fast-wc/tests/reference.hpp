#pragma once

#include <chrono>
#include <string>

#include "count.hpp"

// One byte at a time: slow and obviously right.
inline Counts plain_counts(const std::string& s) {
    Counts c;
    bool in_word = false;
    for (char ch : s) {
        unsigned char u = static_cast<unsigned char>(ch);
        c.lines += u == '\n';
        bool space = is_space(u);
        c.words += !space && !in_word;
        in_word = !space;
    }
    c.bytes = s.size();
    return c;
}

// Text with every kind of whitespace, letters, and bytes above 127.
inline std::string mixed_text(std::size_t n, unsigned seed) {
    static const char pool[] = "ab cd\n\t\v\f\r  x\xc3\xa9y\n\nzz";
    std::string s(n, ' ');
    unsigned x = seed * 2654435761u + 1;
    for (char& c : s) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        c = pool[x % (sizeof pool - 1)];
    }
    return s;
}

template <typename F> double seconds(F f) {
    auto start = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}
