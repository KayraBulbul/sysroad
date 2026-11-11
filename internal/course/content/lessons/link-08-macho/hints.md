`load_commands`: start at 32, and for each command read `cmd` and
`cmdsize`, record where it started, then move on by `cmdsize`:

```cpp
std::size_t at = 32;
for (std::uint32_t i = 0; i < h.ncmds; ++i) {
    LoadCommand c{read_u32(b, at), read_u32(b, at + 4), at};
    if (c.size < 8 || at + c.size > b.size()) {
        throw std::out_of_range("bad load command");
    }
    out.push_back(c);
    at += c.size;
}
```

A size under 8 would loop on the same command forever.
---
For the 16-byte names, take the field and cut it at the first NUL, if
there is one:

```cpp
std::string_view f = b.substr(at, 16);
return std::string(f.substr(0, f.find('\0')));
```

`find` returns `npos` when there's no NUL, and `substr(0, npos)` takes
it all. Section `i` of a segment command at `at` starts at
`at + 72 + 80 * i`.
---
`dylibs`: the name is at `c.offset + read_u32(b, c.offset + 8)`, and
must end before `c.offset + c.size`. `symbols`: entry `i` is at
`symoff + 16 * i`, and its name is at `stroff + n_strx`.
