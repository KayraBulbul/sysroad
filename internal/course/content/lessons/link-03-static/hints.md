A small helper for each field, since they're padded with spaces:

```cpp
std::string_view field(std::string_view header, std::size_t at, std::size_t len) {
    std::string_view f = header.substr(at, len);
    while (!f.empty() && f.back() == ' ') {
        f.remove_suffix(1);
    }
    return f;
}
```

The name is `field(h, 0, 16)` and the size is `field(h, 48, 10)`,
parsed as decimal. Check that `pos + 60 + size` fits before you take
the data. After each member, `pos += 60 + size`, and if that's odd,
skip the padding `'\n'`, which must be there.
---
Deal with the special names first: `/` is the GNU index and `//` the
long-name table (keep both as `string_view`s, they're not members),
`#1/n` is a BSD long name, and any other name starting with `/` is a
GNU long name. Otherwise strip a trailing `/`. GNU puts the `//`
table before any member that uses it, so it's always there by the
time you need it.

Members whose name starts with `__.SYMDEF` are the BSD index, not
members.
---
Keep a `std::map<std::size_t, std::string>` from header offset to
member name as you go, and read the index after the loop, when every
member is known. A 4-byte number:

```cpp
std::uint32_t v = 0;
for (int i = 0; i < 4; ++i) {
    auto byte = static_cast<unsigned char>(b[at + i]);
    v |= std::uint32_t{byte} << (big_endian ? 8 * (3 - i) : 8 * i);
}
```
