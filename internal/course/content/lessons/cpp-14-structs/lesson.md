---
title: Structs and member functions
minutes: 15
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
