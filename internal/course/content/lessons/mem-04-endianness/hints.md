Little-endian is the same shifts in the other direction: the *first*
byte is the least significant.

```cpp
return std::uint32_t(p[0]) | std::uint32_t(p[1]) << 8 |
       std::uint32_t(p[2]) << 16 | std::uint32_t(p[3]) << 24;
```

For 16 bits, cast to `std::uint16_t` at the end:
`return std::uint16_t(p[0] << 8 | p[1]);`. That one is safe without
the inner casts because `0xff << 8` fits easily in an `int`.
---
Get a byte pointer once:
`auto* p = reinterpret_cast<const unsigned char*>(bytes.data());`.
Reading a `char` buffer as `unsigned char` is always allowed.

Check `bytes.size() >= 29` before touching `p[28]`. Then compare the
first 8 bytes: `bytes.substr(0, 8) == std::string_view("\x89PNG\r\n\x1a\n", 8)`.
The explicit length matters. Without it, the string would stop at the
first `\0`, and while PNG's signature has none, the habit saves you on
formats that do.
---
`describe` with `std::format`:

```cpp
std::string s = std::format("{} x {}, {}-bit {}", info.width, info.height,
                            info.bit_depth, colour_name(info.color_type));
if (info.interlaced) s += ", interlaced";
```

where `colour_name` is a `switch` with a `default` of `"unknown"`.
