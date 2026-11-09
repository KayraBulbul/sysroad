---
title: Endianness and reading binary headers
minutes: 20
files: [endian.cpp, png.cpp]
run:
  args: ["logo.png", "notes.txt"]
  files:
    logo.png: !!binary "iVBORw0KGgoAAAANSUhEUgAAAEAAAAAwCAYAAAChS3wfAAAAj0lEQVR4nOXOMQEAAAzDoEiv9MngGApoz6UDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA1o6oKUDWjqgpQNaOqClA9oBE+4BaUw0ttwAAAAASUVORK5CYII="
    notes.txt: "not a picture\n"
deeper:
  - title: PNG specification, chapter 5 (W3C)
    url: https://www.w3.org/TR/png/#5DataRep
  - title: The byte order fallacy (Rob Pike)
    url: https://commandcenter.blogspot.com/2012/04/byte-order-fallacy.html
---
Your machine stores the low byte first (little-endian). Files and
network protocols pick their own order, and many pick **big-endian**,
high byte first. PNG is one of them. So you can't `memcpy` four bytes
of a PNG into a `std::uint32_t` and expect the right number.

The portable way never asks which machine you're on. It reads the
bytes and puts each one where it belongs with shifts:

```cpp
// big-endian: the first byte is the most significant
std::uint32_t v = std::uint32_t(p[0]) << 24 | std::uint32_t(p[1]) << 16 |
                  std::uint32_t(p[2]) << 8 | std::uint32_t(p[3]);
```

The casts matter. `p[0]` is an `unsigned char`, and arithmetic promotes
it to a signed `int`. Without them, `0xde << 24` is a *negative* `int`,
and the moment it meets a wider type it sign-extends: put the result in
a `std::uint64_t` and you get `0xffffffffdeadbeef`. Before C++20, that
shift was undefined behaviour too. Cast first and neither can happen.

A PNG file starts with the same 8 bytes every time, its *signature*:
`89 50 4E 47 0D 0A 1A 0A` (`\x89PNG\r\n\x1a\n`). Then comes the IHDR
chunk, at fixed offsets:

- offset 8, 4 bytes: the chunk length, big-endian, always 13
- offset 12, 4 bytes: the chunk type, `IHDR`
- offset 16, 4 bytes: width, big-endian
- offset 20, 4 bytes: height, big-endian
- offset 24, 1 byte: bit depth
- offset 25, 1 byte: colour type (0 grey, 2 RGB, 3 palette, 4 grey + alpha, 6 RGBA)
- offset 28, 1 byte: interlace, where 1 means interlaced

`main.cpp` is done. It reads each file and prints what `describe` says:

```
$ ./endian logo.png notes.txt
logo.png: 64 x 48, 8-bit RGBA
notes.txt: not a PNG
```

## Assignment

- [ ] The four `load_` functions read 16 and 32-bit numbers in either byte order
- [ ] `parse_png` checks the signature, length and `IHDR`, then reads the fields
- [ ] `describe` prints `W x H, D-bit COLOUR`, plus `, interlaced` when it is
