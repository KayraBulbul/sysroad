`parse_header` is one line per field:

```cpp
if (b.size() < 16 || !b.starts_with("\177ELF")) {
    throw std::runtime_error("not an ELF file");
}
if (b[4] != 2 || b[5] != 1) {
    throw std::runtime_error("only 64-bit little-endian ELF is supported");
}
ElfHeader h;
h.os_abi = static_cast<std::uint8_t>(b[7]);
h.type = read_u16(b, 16);
// … one per row of the table
```

`"\177ELF"` is the magic in octal. `"\x7fELF"` would go wrong: a hex
escape takes every hex digit after it, and `E` is one.
---
`parse_segments`:

```cpp
for (std::size_t i = 0; i < h.phnum; ++i) {
    std::size_t at = h.phoff + i * h.phentsize;
    Segment s;
    s.type = read_u32(b, at);
    s.flags = read_u32(b, at + 4);
    s.offset = read_u64(b, at + 8);
    // vaddr at 16, filesz at 32, memsz at 40, align at 48
    out.push_back(s);
}
```
---
The names are `switch` statements. For the flags:

```cpp
std::string s = "   ";
if (flags & 4) s[0] = 'R';
if (flags & 2) s[1] = 'W';
if (flags & 1) s[2] = 'E';
```
