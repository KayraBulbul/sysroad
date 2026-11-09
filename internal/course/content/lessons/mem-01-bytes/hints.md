Cast once, then index: `auto* b = static_cast<const unsigned char*>(p);`.
A `void*` converts to any object pointer with `static_cast`.
`std::format("{:02x}", b[i])` gives two hex digits. Put the space
*between* bytes, not after the last one.
---
`bits_of` is one line: `return std::bit_cast<std::uint32_t>(f);`
(from `<bit>`). The `std::memcpy` version is also fine:
`std::uint32_t u; std::memcpy(&u, &f, sizeof u); return u;`.
---
Shift the field down to bit 0, then mask off everything above it:

```cpp
std::uint32_t u = bits_of(f);
FloatParts fp;
fp.sign = u >> 31;
fp.exponent = (u >> 23) & 0xff;    // 8 bits
fp.mantissa = u & 0x7fffff;        // 23 bits
```
