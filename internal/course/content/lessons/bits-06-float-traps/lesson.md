---
title: Floats that surprise you
minutes: 20
files: [traps.cpp]
run: {}
deeper:
  - title: "0.30000000000000004.com"
    url: https://0.30000000000000004.com/
  - title: "Comparing floating point numbers (Bruce Dawson)"
    url: https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
  - title: "The Patriot missile failure (Douglas Arnold)"
    url: https://www-users.cse.umn.edu/~arnold/disasters/patriot.html
---
The last lesson explains every float surprise you'll ever meet: a
float has a fixed number of bits, so most results get rounded to the
nearest value it can hold. Python and Go use the same IEEE 754 numbers,
so none of this is new to C++. Here are the five traps that cause most
of the bugs.

**1. Don't compare with `==`.** 0.1, 0.2 and 0.3 are all rounded, each
by a different amount, so `0.1 + 0.2 == 0.3` is false. Ask whether two
results are *close* instead. How close depends on their size: floats
near a million are much further apart than floats near 1. So scale the
tolerance by the numbers themselves:

```cpp
std::abs(a - b) <= 1e-9 * std::max(std::abs(a), std::abs(b))
```

That alone breaks down near zero, where the tolerance shrinks to
nothing, so real code puts a floor under the scale.

**2. Errors add up.** Adding 0.1 ten thousand times adds ten thousand
rounding errors. Keep counts in integers and convert once at the end.
Dividing by 10 is better than multiplying by 0.1, because 10 is exact
and 0.1 isn't.

**3. NaN isn't equal to anything, itself included.** Every comparison
with a NaN is false: `<`, `>`, `==`, all of them. So a NaN slips
through any `if` you wrote for ordinary numbers. Test for it with
`std::isnan(x)`.

**4. Converting to an integer cuts off the fraction.** It doesn't
round. `0.29 * 100` is 28.999999999999996, and as an integer that's
28. Round first with `std::round` or `std::llround`. Better still,
keep money in whole cents from the start.

**5. Big integers don't fit.** A `double` has 53 bits of precision,
so above 2^53 (about 9 × 10^15) it can't hold every whole number.
A 64-bit id that takes a detour through a `double` comes back as a
different id.

`traps.cpp` has one function for each trap, and `traps.hpp` says what
each promises. Nothing crashes this time, and the sanitizers stay
silent. Every bug here just returns a wrong number.

## Assignment

- [ ] `close_enough` compares with a tolerance that scales
- [ ] `elapsed_seconds` doesn't drift
- [ ] `largest` ignores NaNs wherever they are
- [ ] `to_cents` rounds to the nearest cent
- [ ] `parse_id` keeps every digit of a 64-bit id
