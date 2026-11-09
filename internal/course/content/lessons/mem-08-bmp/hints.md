Read everything first and check `ok()` once:

```cpp
ByteReader r(file);
auto magic = r.bytes(2);
if (!r.ok() || magic[0] != 'B' || magic[1] != 'M') throw BmpError("not a BMP file");
r.skip(8);                               // file size, reserved
std::uint32_t offset = r.le32();
std::uint32_t dib = r.le32();
auto width = static_cast<std::int32_t>(r.le32());
auto height = static_cast<std::int32_t>(r.le32());
r.skip(2);                               // planes
int bits = r.le16();
std::uint32_t compression = r.le32();
r.skip(20);                              // the rest of the 40-byte header
if (!r.ok()) throw BmpError("truncated header");
```
---
Two numbers can overflow. `-height` is undefined when `height` is
`INT32_MIN`, because +2147483648 doesn't fit in an `int32_t`. Reject it
along with 0 before negating. And `bits * width` or
`row_size * height` can pass 4 billion. Do that maths in
`std::uint64_t`:

```cpp
std::uint64_t row = (std::uint64_t(bits) * width + 31) / 32 * 4;
if (offset + row * h > file.size()) throw BmpError("pixel data is truncated");
```
---
In `pixel_at`, pick the row from the file's point of view, then the
column:

```cpp
std::size_t row = info.top_down ? y : info.height - 1 - y;
std::size_t at = info.pixel_offset + row * info.row_size + x * (info.bits / 8);
return {file[at + 2], file[at + 1], file[at]};   // B, G, R in the file
```
---
In `main`, `std::format("#{:02x}{:02x}{:02x}", p.r, p.g, p.b)` prints
the colour. Parse `X` and `Y` with `std::from_chars` (or `std::stoi` in
a `try`), and treat junk as a usage error: print a usage line and
return 2.
