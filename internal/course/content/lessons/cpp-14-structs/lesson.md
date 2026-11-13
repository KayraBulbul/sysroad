---
title: Structs and member functions
minutes: 20
files: [histogram.cpp]
run:
  stdin: "hello, world\n"
deeper:
  - title: Classes (cppreference)
    url: https://en.cppreference.com/w/cpp/language/classes
---
A struct can hold functions as well as data. A **member function** runs on
one particular object, and it sees that object's fields by name:

```cpp
struct Counter {
    int n = 0;
    void bump() { ++n; }               // changes this object's n
    int value() const { return n; }    // const: only looks
};

Counter c;
c.bump();
c.value();  // 1
```

The `const` after `value()` promises the function won't change the
object. Lesson 1.3.3 is all about why that matters. For now: functions that
only look get a `const`.

You can declare members in the header and define them in the `.cpp`,
prefixed with the struct's name:

```cpp
int Counter::value() const { return n; }
```

This lesson builds a **byte histogram**: how many times each of the 256
possible byte values appears. You'll use it again when you look at file
formats. Its data is one fixed-size array, already in `histogram.hpp`:

```cpp
std::array<std::size_t, 256> counts{};  // {} sets every count to 0
```

**Bytes in hex.** Programmers write bytes in **hexadecimal**, base 16.
The digits are 0–9, then a–f for 10–15, and a `0x` prefix marks the
number as hex. One hex digit is 4 bits, so any byte (0–255) fits in
exactly two digits: `0x6c` is 6×16 + 12 = 108, the code for `'l'`, and
`0xff` is 255. Phase 4 covers binary and hex properly. For now, let
`std::format` do the conversion:

```cpp
std::format("{:x}", 108);    // "6c"   x: lowercase hex
std::format("{:02x}", 5);    // "05"   2: at least 2 wide, 0: pad with zeros
std::format("{:02x}", 255);  // "ff"
```

It's Python's `f"{n:02x}"` and Go's `fmt.Sprintf("%02x", n)`. Without the
`0`, the padding is spaces, so 5 would print as `" 5"`.

Mind the type when you leave the `x` out: `{}` prints a `char` as a
character (`l`), but an `unsigned char` as a number (`108`).

**The spec**

- `add(bytes)` counts every byte in `bytes`. It can be called many times,
  and the counts keep adding up.
- `count(b)` is how many times byte `b` has been seen, and `total()` is
  every byte so far.
- `most_common()` is the byte seen most often. On a tie, the smallest byte
  value wins. With nothing added yet, it's 0.
- `main` feeds all of stdin through `add` and prints
  `most common: 0x6c (3 of 13)`, with the byte as two lowercase hex digits.

## Assignment

- [ ] Implement `add`, `count` and `total`
- [ ] Implement `most_common`, smallest byte on a tie
- [ ] Print the summary line in `main`
