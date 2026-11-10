---
title: Integer promotions and conversions
minutes: 20
files: [conv.cpp]
run: {}
deeper:
  - title: Implicit conversions (cppreference)
    url: https://en.cppreference.com/w/cpp/language/implicit_conversion
  - title: "Usual arithmetic conversions (cppreference)"
    url: https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions
  - title: "std::cmp_less and friends (cppreference)"
    url: https://en.cppreference.com/w/cpp/utility/intcmp
---
In Go, `int32 + int64` doesn't compile. You write the conversion and
you see it. C++ inherited C's rules instead: it converts silently, and
the results are a steady source of real bugs. There are only a few
rules, and once you know them the bugs are easy to spot.

**1. Small types become `int` first.** Before any arithmetic, `char`,
`short`, `bool` and the 8 and 16-bit types are *promoted* to `int`.
There is no such thing as 8-bit addition in C++:

```cpp
std::uint8_t a = 200, b = 100;
auto sum = a + b;          // an int, 300
std::uint8_t s = a + b;    // 300 narrowed to 8 bits: 44
```

**2. Signed meets unsigned, unsigned wins.** When the two sides of an
operator have different types, the "smaller" one converts to the
other. For an `int` and an `unsigned`, the `int` converts, so a
negative number turns into a huge one:

```cpp
-1 < 5u       // false: -1 becomes 4294967295
```

`std::size_t` is unsigned, so this rule is in play every time you
compare an `int` with `.size()`, and `v.size() - 1` on an empty vector
is 18446744073709551615, not -1. An unsigned value is also always
`>= 0`, so a countdown loop on one never ends by itself.

**3. The expression decides the type, not the destination.** The
right-hand side is computed first, in the type its operands give it,
and only then converted to fit the variable:

```cpp
std::uint32_t a = 3'000'000'000;
std::uint64_t wide = a * 2;                   // 32-bit multiply wraps, then widens
std::uint64_t right = std::uint64_t{a} * 2;   // widen first
```

A literal like `1` is an `int`. `1u`, `1ull` or `std::uint64_t{1}` say
otherwise.

**4. `char` is signed on both your machines.** A byte of `0xe9` in a
`char` is -23. Convert to `unsigned char` before you use a character
as a number or an index.

`conv.cpp` has six short functions, each with one of these bugs, and
`conv.hpp` says what each one promises. The compiler warns about one
of them. UBSan and ASan catch some of the others at run time, and a
few just quietly return the wrong number.

## Assignment

- [ ] `below` compares an `int` with an `unsigned` as numbers
- [ ] `last_index_of` counts down without wrapping around
- [ ] `byte_histogram` counts bytes above 127 too
- [ ] `average` doesn't lose the ninth bit
- [ ] `mib_to_bytes` multiplies in 64 bits
- [ ] `bit` works for all 64 positions
