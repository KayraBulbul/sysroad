---
title: Aliasing and other optimisation blockers
minutes: 35
files: [blockers.cpp]
program: blockers
optimize: true
run: {}
deeper:
  - title: "CS:APP 5.1-5.6: optimisation blockers, and eliminating them"
    url: https://csapp.cs.cmu.edu/
  - title: "Strict aliasing rules (cppreference: reinterpret_cast, Type aliasing)"
    url: https://en.cppreference.com/w/cpp/language/reinterpret_cast#Type_aliasing
  - title: "Optimizing for the char* aliasing problem (Travis Downs)"
    url: https://travisdowns.github.io/blog/2019/08/26/vector-inc.html
---
The optimiser is only allowed to make changes it can prove don't
change what your program does. When it can't prove something, it
plays safe. Three things it often can't see past:

**A function call it can't see into.** In

```cpp
for (std::size_t i = 0; i < std::strlen(s); ++i) { … s[i] = … }
```

`strlen` runs once per iteration, walking the whole string each
time, so the loop is quadratic. Could the compiler call it once? Only
if it knew the loop body never changes the string's length, and the
body writes to `s`. Writing `'a'` over `'A'` can't change the length,
but proving that is beyond it. You know, so hoist the call.

**Memory that might be the same memory.** Two pointers, or a pointer
and a reference, may refer to overlapping memory: they **alias**.
Here:

```cpp
for (std::size_t i = 0; i < img.size; ++i) {
    img.pixels[i] = 255 - img.pixels[i];
}
```

`img.pixels[i]` is a `std::uint8_t`. C++ lets `char`, `unsigned char`
and `std::byte` (and `std::uint8_t`, which is `unsigned char`) point at
*any* object, including `img` itself. So as far as the compiler knows,
the store to `img.pixels[i]` might have just changed `img.size` or
`img.pixels`, and it has to reload both from memory on every
iteration. That rules out vectorising, which needs to know up front
how many bytes there are and where.

The cure is to copy what the loop reads into local variables first.
Nothing can point at a local whose address was never taken, so the
compiler is free to keep it in a register:

```cpp
std::uint8_t* p = img.pixels;
std::size_t n = img.size;
for (std::size_t i = 0; i < n; ++i) { p[i] = … }
```

**The same thing, hidden in a class.** `pixels.size()` and
`pixels[i]` on a `std::vector<std::uint8_t>` read the vector's begin
and end pointers, and a byte store might change those too. A
range-`for` reads `begin()` and `end()` once, before the loop starts,
which is exactly the copy you'd write by hand.

Each function in `blockers.cpp` is correct and slow. Keep what they do
and remove what blocks the optimiser. Run races your versions against
the originals at `-O2`.

## Assignment

- [ ] `lowercase` calls `strlen` once
- [ ] `invert` reads `img.pixels` and `img.size` into locals before the loop
- [ ] `threshold` doesn't call `size()` or `operator[]` through the vector inside the loop
- [ ] Run it. Which fix bought the most, and why?
