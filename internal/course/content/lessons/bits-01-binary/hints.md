`to_binary`: walk the bit numbers from 31 down to 0. Bit `i` of `x` is
`(x >> i) & 1`. Add a space before a digit when `i % 4 == 3`, except at
the very start.
---
`parse_number`: look at the prefix first to pick the base (`0x` → 16,
`0b` → 2, otherwise 10) and cut it off with `s.remove_prefix(2)`. No
digits left means it wasn't a number.

Then turn each character into a digit: `'0'..'9'` is `c - '0'`,
`'a'..'f'` is `c - 'a' + 10`, and the same for capitals. A digit that
is `>= base` is an error.
---
The overflow check in `parse_number`: do the arithmetic in a
`std::uint64_t`, where one more step can't overflow, and give up as
soon as the value is above `0xffffffff`:

```cpp
std::uint64_t value = 0;
for (char c : s) {
    // ...work out digit...
    value = value * base + digit;
    if (value > 0xffffffff) return std::nullopt;
}
```
---
`count_ones`: either test each of the 32 bits, or use the classic
trick: `x & (x - 1)` clears the lowest 1 bit of `x`. Count how many
times you can do that before `x` is 0.

The same trick gives `is_power_of_two`: a power of two has exactly one
1 bit, so clearing it leaves 0. Remember that 0 isn't a power of two.
