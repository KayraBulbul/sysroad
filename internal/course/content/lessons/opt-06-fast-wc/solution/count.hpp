#pragma once
// Shared by your code, wc.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <cstring>

struct Counts {
    std::size_t lines = 0;  // '\n' bytes
    std::size_t words = 0;  // runs of bytes that aren't whitespace
    std::size_t bytes = 0;
};

// Counting carries on from one chunk to the next: a word can start in one
// chunk and end in the next.
struct CountState {
    Counts counts;
    bool in_word = false;  // whether the last byte seen was part of a word
};

// Whitespace is the six ASCII bytes ' ', '\t', '\n', '\v', '\f' and '\r'
// (9 to 13, and 32), as std::isspace has them in the C locale.
inline bool is_space(unsigned char c) {
    return c == ' ' || (c >= '\t' && c <= '\r');
}

// Adds the lines, words and bytes of p[0, n) to st.
void count_chunk(CountState& st, const char* p, std::size_t n);

// Reads fd to the end and counts it. Returns false if a read fails (with
// errno set), and the counts so far are in out.
bool count_fd(int fd, Counts& out);

// From lesson 5.2.4.
using u8x16 = std::uint8_t __attribute__((vector_size(16)));

inline u8x16 load16(const char* p) {
    u8x16 v;
    std::memcpy(&v, p, sizeof v);
    return v;
}

inline u8x16 splat16(std::uint8_t b) {
    return u8x16{} + b;
}
