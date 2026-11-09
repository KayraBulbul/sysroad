**"Network byte order" is big-endian.** IP addresses and port numbers
in packet headers are big-endian, which is why C has `htonl`/`ntohl`
("host to network long"). On a big-endian machine they do nothing, and
on yours they swap bytes. C++23 adds `std::byteswap`, and C++20 has
`std::endian::native` to ask which machine you're on. The shift
approach in this lesson never needs either.

**The compiler sees through it.** At `-O2`, clang turns the four
shifts and ORs of `load_le32` into a single `mov` on x86-64, and
`load_be32` into `mov` + `bswap`. Portable code costs nothing. Check it
on Compiler Explorer (godbolt.org) once you get to the assembly labs in
phase 4.

**Why PNG's signature looks like that.** Each byte catches a different
way a file gets mangled. `\x89` catches 7-bit transfers that clear the
high bit, `\r\n` catches DOS↔Unix newline conversion, `\x1a` stops
`type` on DOS from printing binary, and the final `\n` catches the
reverse conversion.

**Chunks carry a CRC.** Every PNG chunk ends with a CRC-32 of its type
and data. This lesson skips the check, but `zlib`'s `crc32` would do it
in one call.

**`file` does this for a living.** `file logo.png` reads the same
header. Its magic database (`/usr/share/file/misc/magic`) is thousands
of these signatures.
