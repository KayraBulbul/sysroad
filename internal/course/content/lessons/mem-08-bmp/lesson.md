---
title: Read a BMP image header by hand
minutes: 45
files: [bmp.cpp, main.cpp]
run:
  args: ["flag.bmp", "2", "0"]
  files:
    flag.bmp: !!binary "Qk1mAAAAAAAAADYAAAAoAAAABQAAAAMAAAABABgAAAAAADAAAAATCwAAEwsAAAAAAAAAAAAApjkApjkApjkApjkApjkAAP///////////////////wAeK9UeK9UeK9UeK9UeK9UA"
deeper:
  - title: BMP file format (Wikipedia)
    url: https://en.wikipedia.org/wiki/BMP_file_format
  - title: BITMAPINFOHEADER (Microsoft Learn)
    url: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
---
Time to use the whole chapter on a real format. BMP is Windows' plain
bitmap: a couple of headers, then raw pixels. All its numbers are
**little-endian**, and two of them are **signed**.

The first 54 bytes, which your `ByteReader` reads in order:

- `BM`, then the file size (u32), 4 reserved bytes, and the **offset of
  the pixel data** (u32)
- the info header's size (u32): 40, or 108 or 124 for newer versions,
  which add fields after the first 40 bytes
- width (i32) and height (i32). A **negative height means the rows are
  stored top-down**
- planes (u16, always 1), **bits per pixel** (u16) and **compression**
  (u32: 0 means none, 3 means "bitfields", which you can treat as none
  for 32-bit images)

`le32()` returns a `std::uint32_t`, so the signed fields need
`static_cast<std::int32_t>(r.le32())`.

The pixels start at the offset from the header, which isn't always 54.
Rows are stored **bottom row first** unless the height was negative,
each pixel is **B, G, R** (plus a fourth byte at 32 bits), and every row
is padded to a multiple of 4 bytes:

```cpp
row_size = (bits * width + 31) / 32 * 4;   // 5 pixels × 3 bytes = 15 → 16
```

Your program prints a summary, or one pixel's colour when given `X Y`,
where (0, 0) is the top-left corner:

```
$ ./bmp flag.bmp
flag.bmp: 5 x 3, 24-bit, bottom-up, 16 bytes per row (1 of padding)
$ ./bmp flag.bmp 2 0
#d52b1e
$ ./bmp notes.txt
bmp: notes.txt: not a BMP file
```

A file is input you don't control. Treat every number in it as a
claim to check, never as a fact. `bmp.hpp` lists the exact error
messages.

## Assignment

- [ ] `parse_bmp` reads both headers with a `ByteReader` and fills in `BmpInfo`
- [ ] It throws `BmpError` for every case in `bmp.hpp`, including pixel data that's shorter than the header claims
- [ ] `pixel_at` finds a pixel's bytes, flipping bottom-up rows
- [ ] `main` prints the summary or one pixel, and errors as `bmp: FILE: MESSAGE` with exit status 1
