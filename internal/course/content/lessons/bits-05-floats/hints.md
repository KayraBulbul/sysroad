Pull the fields out once, the same way in all three functions:

```cpp
bool negative = bits >> 31;
int exponent = (bits >> 23) & 0xff;
std::uint32_t fraction = bits & 0x7fffff;
```

`classify` is then two questions: is `exponent` 0, 255 or something
else, and is `fraction` zero?
---
`value_of` for a normal number: put the hidden 1 back with
`fraction | 0x800000`. That's a 24-bit whole number, 2^23 times too
big, so the power drops by 23:
`std::ldexp(double(fraction | 0x800000), exponent - 127 - 23)`.

A subnormal is `std::ldexp(double(fraction), -126 - 23)`. Multiply by
-1.0 when the sign bit is set, which also turns 0.0 into -0.0.
`<limits>` has `std::numeric_limits<double>::infinity()` and
`quiet_NaN()`.
---
`from_uint`: 0 is all zero bits. Otherwise find the highest set bit,
`int top = 31 - std::countl_zero(n);` (from `<bit>`). The exponent
field is `127 + top`.

If `top <= 23`, the number fits exactly: shift it left so its highest
bit lands on bit 23, and mask that bit off to get the fraction.
---
When `top > 23` there are `drop = top - 23` bits too many. Split `n`:

```cpp
std::uint32_t kept = n >> drop;               // 24 bits, the hidden 1 on top
std::uint32_t rest = n & ((1u << drop) - 1);  // the bits that fall off
std::uint32_t half = 1u << (drop - 1);
```

Round up (`++kept`) when `rest > half`, or when `rest == half` and
`kept` is odd. One more case: rounding up `1111…1` gives `kept ==
1u << 24`, a 25-bit number. Then the value is the next power of two:
add 1 to the exponent and the fraction is 0.
