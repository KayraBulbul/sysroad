---
title: Lines, words and bytes from a stream
minutes: 20
files: [wc.cpp]
run:
  stdin: "the quick brown fox\njumps over\nthe lazy dog\n"
deeper:
  - title: std::basic_istream::read (cppreference)
    url: https://en.cppreference.com/w/cpp/io/basic_istream/read
  - title: std::format (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/format/format
---
Real `wc` with no arguments counts its input and prints three numbers:

```text
$ printf 'hello world\n' | ./wc
      1       2      12
```

Input can be bigger than memory, so read it in **chunks**:

```cpp
std::array<char, 4096> buf;
while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
    std::string_view chunk(buf.data(), in.gcount());
    // ... look at chunk
}
```

`in.read` fills the buffer. It reports failure on the last, partial
chunk, which is why the loop also checks `in.gcount()`, the number of bytes
it actually read.

The catch is that a word can be split across two chunks. Calling last
lesson's `count_words` on each chunk counts `"fo" | "x"` as two words.
Keep the in-word state *across* chunks instead.

`Counts` is declared in `wc.hpp`, so your code and the tests agree on it.
Headers are how C++ shares declarations between files. Chapter 1.3 covers
them properly.

**The spec**

- `lines` is the number of `'\n'` bytes. A last line with no `'\n'` isn't
  counted, same as GNU.
- `words` follows last lesson's rule, across the whole stream.
- `bytes` is the total number of bytes read.
- `main` counts `std::cin` and prints each number right-aligned in a
  7-wide column, separated by single spaces, then `'\n'`.
  `std::format("{:>7}", n)` right-aligns.

## Assignment

- [ ] Implement `count_stream` with 4096-byte reads
- [ ] Carry the in-word state across chunks
- [ ] In `main`, count `std::cin` and print the three columns
