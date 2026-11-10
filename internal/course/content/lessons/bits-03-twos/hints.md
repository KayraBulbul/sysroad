`sign_extend`: first keep only the low `bits` bits (careful with
`bits == 32`, like in the last lesson). Then look at the sign bit,
`1u << (bits - 1)`. If it's set, the value is the unsigned reading
minus 2^bits.

There's a neat version with no branch: with `m = 1u << (bits - 1)`,
the answer is `(x ^ m) - m`, done in unsigned and converted to
`std::int32_t` at the end.
---
`add_overflows`: the simplest way is to add in a type that has room:

```cpp
std::int64_t sum = std::int64_t(a) + b;
return sum > INT32_MAX || sum < INT32_MIN;
```

`<cstdint>` has the `INT32_MAX` and `INT32_MIN` constants.
---
`wrapping_add`: unsigned arithmetic wraps, so do the addition there
and convert back: `std::int32_t(std::uint32_t(a) + std::uint32_t(b))`.
The bits are the same ones a signed `add` would produce.
---
`saturating_add` and `checked_mul` both follow the 64-bit pattern.
Compute the exact result in a `std::int64_t`, then compare it with
the two limits. `saturating_add` clamps it (`std::clamp` works) and
`checked_mul` returns `std::nullopt` when it's out of range.
