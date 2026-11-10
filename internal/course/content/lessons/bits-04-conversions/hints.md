`below`: a negative `x` is below every unsigned limit, so handle that
first. After that, `x` is safe to convert:
`return x < 0 || static_cast<unsigned>(x) < limit;`.

C++20 also has `std::cmp_less(x, limit)` in `<utility>`, which compares
any two integers as numbers.
---
`last_index_of`: the idiom for counting an unsigned index down to 0 is

```cpp
for (std::size_t i = v.size(); i-- > 0;) {
    // i goes size-1, size-2, ..., 0
}
```

The test `i-- > 0` compares first and decrements after, so the body
never sees a wrapped value, and an empty vector skips the loop.
---
`byte_histogram`: index with `static_cast<unsigned char>(c)`. Or loop
with `for (unsigned char c : s)`, which converts each character as it
is read.
---
`average`: `a + b` is already an `int`, and 300 fits in an `int`. The
bug is storing it in a `std::uint8_t` before dividing. Divide first:
`return static_cast<std::uint8_t>((a + b) / 2);`.
---
`mib_to_bytes` and `bit`: make the *first* operand 64 bits wide, and
the rest of the expression follows: `std::uint64_t{mib} * 1024 * 1024`
and `std::uint64_t{1} << n`.
