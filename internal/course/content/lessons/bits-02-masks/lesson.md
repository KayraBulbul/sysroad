---
title: "Masks, shifts and bit fields"
minutes: 20
files: [fields.cpp]
run:
  args: ["rgb", "0xfd20"]
deeper:
  - title: "High color: RGB565 (Wikipedia)"
    url: https://en.wikipedia.org/wiki/High_color
  - title: "File-system permissions: notation (Wikipedia)"
    url: https://en.wikipedia.org/wiki/File-system_permissions#Notation_of_traditional_Unix_permissions
---
Real formats pack several small numbers into one word to save space. A
16-bit pixel in the **RGB565** format, used by small displays and old
game consoles, holds three colour channels:

```
bit  15 14 13 12 11 | 10  9  8  7  6  5 |  4  3  2  1  0
      r  r  r  r  r |  g  g  g  g  g  g |  b  b  b  b  b
```

To work with one field you need a **mask**: a number with 1s exactly
where the field is. Build one from a width with a shift and a
subtraction: `(1u << 5) - 1` is `0b11111`, five 1s. Then the four moves
you'll use for the rest of your life:

```cpp
unsigned green = (pixel >> 5) & 0x3f;       // read:   shift down, mask off the rest
pixel |= 1u << 4;                           // set:    OR with the mask
pixel &= ~(1u << 4);                        // clear:  AND with the inverted mask
pixel ^= 1u << 4;                           // toggle: XOR with the mask
if (pixel & (1u << 4)) { /* bit 4 is set */ }
```

Writing a field is a clear followed by a set: wipe the old bits with
`& ~(mask << lo)`, then OR in the new value, shifted into place and
masked so it can't spill into its neighbours.

One trap: shifting a 32-bit value by 32 or more is undefined behaviour,
as you saw in lesson 2.3.6. `(1u << width) - 1` breaks for a field that
is the whole word, so that width needs its own case.

Unix file permissions are nine flag bits, one each for read, write and
execute, for the owner, the group and everyone else. `chmod 754` is
octal for `111 101 100`, which `ls -l` shows as `rwxr-xr--`.

`main.cpp` is done:

```
$ ./fields rgb 0xfd20
r 255  g 166  b 0
$ ./fields pack 255 166 0
0xfd20
$ ./fields mode 754
rwxr-xr--
```

A 5-bit channel only goes up to 31, and a screen wants 0 to 255. Just
shifting left by 3 would turn the brightest red, 31, into 248 instead
of 255. The usual fix copies the top bits into the gap at the bottom:
`(r5 << 3) | (r5 >> 2)`. That maps 0 to 0 and 31 to 255, with
everything in between spread evenly.

## Assignment

- [ ] `get_field` reads `width` bits starting at bit `lo`
- [ ] `set_field` replaces them and leaves every other bit alone
- [ ] `unpack565` and `pack565` convert a pixel to 8-bit channels and back
- [ ] `mode_string` turns nine permission bits into `rwxr-xr--`
