`string_at`: take the section's bytes as `b.substr(s.offset, s.size)`
(after checking they're inside `b`), then `find('\0', at)` from the
offset. No NUL before the end of the section is an error too.
---
`parse_sections` reads in two passes. Keep the `sh_name` offsets in a
vector while reading the headers, since `Section` only has room for
the finished name:

```cpp
std::size_t at = h.shoff + i * h.shentsize;
name_offsets.push_back(read_u32(b, at));
s.type = read_u32(b, at + 4);
s.flags = read_u64(b, at + 8);
// …
```

then, when every header is read, `out[i].name = string_at(b,
out[h.shstrndx], name_offsets[i])`. Check `h.shstrndx` is a real
section number first. Copy `out[h.shstrndx]` before the loop: the
loop changes `out`, and a reference into it could see a half-updated
entry.
---
`parse_symbols`: find the section with `type == 2`, and its string
table `sections[s.link]`. Then step through its bytes:

```cpp
for (std::size_t at = s.offset; at + 24 <= s.offset + s.size; at += 24) {
    Symbol sym;
    sym.name = string_at(b, strtab, read_u32(b, at));
    auto info = static_cast<std::uint8_t>(b.at(at + 4));
    sym.type = info & 0xf;
    sym.bind = info >> 4;
    sym.shndx = read_u16(b, at + 6);
    sym.value = read_u64(b, at + 8);
    sym.size = read_u64(b, at + 16);
    out.push_back(sym);
}
```
