Index the array with the byte as an `unsigned char`, so 0xff is 255 and
not -1:

```cpp
for (char ch : bytes) ++counts[static_cast<unsigned char>(ch)];
```
---
For `most_common`, loop `for (int b = 1; b < 256; ++b)` and keep the best
byte so far. Replace it only when a count is *strictly* greater, so ties
keep the smaller byte. `total()` can add up the array, or you can keep a
running total, as long as it's always right.
---
`std::format("most common: 0x{:02x} ({} of {})\n", b, h.count(b), h.total())`.
Pass `b` as an `int` (or `unsigned`), because `{:x}` on a `char` prints a
character, not a number.
