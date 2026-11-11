All three readers are one loop with a different length:

```cpp
std::uint64_t v = 0;
for (std::size_t i = 0; i < n; ++i) {
    v |= std::uint64_t{static_cast<unsigned char>(b[at + i])} << (8 * i);
}
```

Cast to `unsigned char` first: a `char` of `0xff` is -1 on x86-64,
and would fill the top bits with ones. Check the bounds as
`at > b.size() || b.size() - at < n`: `at + n` can wrap around when
`at` is huge.
---
For ELF, check the magic and that there are at least 6 bytes, then
look at `b[4]` and `b[5]`. For Mach-O, read the first 4 bytes with
`read_u32` and `switch` on it: `0xfeedfacf` and `0xfeedface` are
little-endian files; the byte-swapped values `0xcffaedfe` and
`0xcefaedfe` are big-endian ones.
---
`ca fe ba be` read with `read_u32` is `0xbebafeca`. For the next four
bytes you need big-endian: read them with `read_u32` and swap the
bytes, or build the number with `b[4]` as the most significant byte.
A count from 1 to 19 is a universal binary; anything else is a Java
class.
