`close_enough`: start with `if (a == b) return true;`, which covers
equal infinities. If either one is still infinite after that
(`std::isinf`), they aren't close. Then:

```cpp
double scale = std::max({std::abs(a), std::abs(b), 1.0});
return std::abs(a - b) <= 1e-9 * scale;
```

The `1.0` is the floor. A NaN makes `a - b` a NaN, and comparing with
that is false, which is the answer you want.
---
`elapsed_seconds` needs no loop at all: `return ticks / 10.0;`.
Writing `10.0` makes it a floating-point division. `ticks * 0.1` is
close, but 0.1 is already rounded, so some results come out one step
off: `3 * 0.1` is 0.30000000000000004.
---
`largest`: the bug is that a NaN in `v[0]` becomes `best`, and then
`x > best` is false for every `x`. Start `best` as NaN and take `x`
when it isn't a NaN and either `best` still is one or `x > best`.
---
`to_cents`: `return std::llround(dollars * 100);`. It rounds halfway
cases away from zero and returns a `long long`.
---
`parse_id`: never go through a `double`. `std::from_chars` in
`<charconv>` reads straight into an integer and reports overflow:

```cpp
std::uint64_t value = 0;
const char* end = digits.data() + digits.size();
auto [stop, error] = std::from_chars(digits.data(), end, value);
```

It succeeded if `error == std::errc{}`, and it used the whole string
if `stop == end`.
