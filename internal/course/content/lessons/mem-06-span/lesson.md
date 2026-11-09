---
title: "Pointer arithmetic and std::span"
minutes: 20
files: [reader.cpp]
run:
  args: ["scores.bin"]
  files:
    scores.bin: !!binary "U0NPUgEAAwADYWRhDS8AAAVncmFjZY4mAAAFbGludXMAAgAA"
deeper:
  - title: std::span (cppreference)
    url: https://en.cppreference.com/w/cpp/container/span
  - title: Pointer arithmetic (cppreference)
    url: https://en.cppreference.com/w/cpp/language/operator_arithmetic#Additive_operators
---
Adding to a pointer moves it by whole elements, not bytes. If `p` is an
`int*`, `p + 3` is 12 bytes further on. `p[3]` is just shorthand for
`*(p + 3)`, and subtracting two pointers into the same array gives the
number of elements between them (a `std::ptrdiff_t`).

The rule that bites: a pointer may only point *into* its array or one
past the end. Computing `p + 100` on a 10-element array is undefined
behaviour even if you never read through it. The compiler is allowed to
assume it never happens, so a check like `if (p + n > end)` can be
optimised away exactly when `n` is huge. Compare *lengths* instead:
`if (n > size - pos)`.

`std::span<T>` (C++20) is a pointer and a length in one small value,
the bundle the last lesson kept passing by hand. It's a **view**: it
doesn't own anything, and copying it copies two words, not the data.

```cpp
std::vector<unsigned char> file = read_file("scores.bin");
std::span<const unsigned char> all(file);
auto header = all.first(8);        // the first 8 bytes
auto rest = all.subspan(8);        // everything after them
auto name = rest.subspan(1, 5);    // 5 bytes, starting at index 1
```

`subspan` doesn't check its arguments, so going past the end is
undefined behaviour. Check against `size()` first.

You'll use a span to build a `ByteReader`: a cursor over a binary file
that reads numbers in order and **remembers if any read ran off the
end**. Code that reads a header can then do all its reads and check
`ok()` once at the end. `main.cpp` uses it to read a small file of game
scores:

```
$ ./reader scores.bin
version 1, 3 players
ada         12045
grace        9870
linus         512
```

## Assignment

- [ ] `bytes(n)` returns the next `n` bytes as a subspan and moves past them
- [ ] A read past the end returns nothing, clears `ok()` and doesn't move
- [ ] `u8`, `le16`, `le32`, `be16` and `be32` read numbers using `bytes`
- [ ] `skip` and `seek` move the cursor, with the same end check
