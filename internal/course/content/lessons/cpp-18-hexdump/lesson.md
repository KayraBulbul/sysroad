---
title: hexdump -C, line by line
minutes: 20
files: [hexdump.cpp]
run:
  stdin: "Hello world, from hexdump!\n"
deeper:
  - title: hexdump(1) man page
    url: https://man7.org/linux/man-pages/man1/hexdump.1.html
  - title: std::format specification (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/format/spec
---
`hexdump -C` is how you look at what's really inside a file:

```text
$ printf 'Hello world\n' | hexdump -C
00000000  48 65 6c 6c 6f 20 77 6f  72 6c 64 0a              |Hello world.|
0000000c
```

Each line has:

1. the **offset** of its first byte, as 8 lowercase hex digits
2. two spaces, then up to 16 bytes as two hex digits and a space each,
   with **one extra space after the 8th**
3. padding (three spaces per missing byte), so a short last line keeps the
   `|` lined up
4. ` |`, the bytes as text (anything from `0x20` to `0x7e` as itself,
   everything else as `.`), then `|`

After the last line comes the total length, as one more 8-digit offset. If
there was no input at all, there's no output at all.

**A class with state.** Input arrives in chunks of any size, from several
files in a row. hexdump treats them all as **one stream**, so lines don't
restart at a file boundary. `Dumper` collects bytes in `pending_` and
prints a line whenever 16 are waiting:

```cpp
Dumper d(std::cout);
d.feed("Hello ");   // nothing printed yet
d.feed("world\n");  // still under 16 bytes
d.finish();         // the partial line, then 0000000c
```

Its fields are `private`: only `Dumper`'s own member functions can touch
them, so nobody can put `pending_` and `offset_` out of step. The
constructor's `: out_(out)` is a **member initialiser list**. A reference
member has to be bound there, since it can't be bound later.

Real hexdump also squeezes repeated lines into a `*`. That's next lesson,
and this lesson's tests don't repeat lines.

## Assignment

- [ ] Implement `hex_line`
- [ ] Implement `Dumper::feed`: buffer bytes, print every full line
- [ ] Implement `Dumper::finish`: the partial line and the final offset
