Write one small helper and use it in both field functions:

```cpp
static std::uint32_t mask_of(int width) {
    return width >= 32 ? 0xffffffffu : (1u << width) - 1;
}
```

Then `get_field` is `(x >> lo) & mask_of(width)`.
---
`set_field` in two steps:

```cpp
std::uint32_t mask = mask_of(width) << lo;   // 1s where the field is
return (x & ~mask) | ((value << lo) & mask);
```

The final `& mask` is what stops a too-large `value` from changing the
bits above the field.
---
`unpack565`: pull the three fields out with `get_field(pixel, 11, 5)`,
`get_field(pixel, 5, 6)` and `get_field(pixel, 0, 5)`. Widen the 5-bit
ones with `(v << 3) | (v >> 2)` and the 6-bit one with
`(v << 2) | (v >> 4)`.

`pack565` goes the other way and just drops the low bits:
`(r >> 3) << 11 | (g >> 2) << 5 | (b >> 3)`.
---
`mode_string`: loop `i` from 8 down to 0. The letter for bit `i` is
`"xwr"[i % 3]`, or think of it as `"rwxrwxrwx"[8 - i]`. Add the letter
if `mode & (1u << i)` is non-zero, and `'-'` if not.
