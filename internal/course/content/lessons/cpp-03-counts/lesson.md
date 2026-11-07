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

**What you return.** `count_stream` returns a `Counts`, a struct with
three numbers that start at zero:

```cpp
struct Counts {
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t bytes = 0;
};
```

It lives in `wc.hpp`, the read-only tab next to `wc.cpp`, so your code and
the tests agree on it. `#include "wc.hpp"` at the top of `wc.cpp` pulls it
in. Headers are how C++ shares declarations between files. Chapter 1.3
covers them properly.

**Formatting output.** `std::format` from `<format>` builds a
`std::string`. Each `{}` is filled in with the next argument, much like Go's
`fmt.Sprintf`:

```cpp
std::string s = std::format("{} has {} legs", "spider", 8);
// "spider has 8 legs"
```

Put a spec after a colon to control the layout. `{:>5}` means "right-align
(`>`) in a field 5 characters wide", padding with spaces on the left:

```cpp
std::cout << std::format("[{:>5}]\n", 42);     // [   42]
std::cout << std::format("[{:>5}]\n", 12345);  // [12345]
std::cout << std::format("[{:<5}]\n", 42);     // [42   ]  (< aligns left)
```

A value wider than the field is never cut off. It just pushes the line
wider. Right-aligning every number to the same width is what lines up
wc's columns.

**The spec**

- `lines` is the number of `'\n'` bytes. A last line with no `'\n'` isn't
  counted, same as GNU.
- `words` follows last lesson's rule, across the whole stream.
- `bytes` is the total number of bytes read.
- `main` counts `std::cin` and prints lines, words and bytes, each
  right-aligned in a 7-wide column, separated by single spaces, then
  `'\n'`.

## Assignment

- [ ] Implement `count_stream` with 4096-byte reads
- [ ] Carry the in-word state across chunks
- [ ] In `main`, count `std::cin` and print the three columns
