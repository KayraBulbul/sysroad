`lowercase`: call `std::strlen(s)` before the loop, keep the answer in
a `std::size_t n`, and loop while `i < n`.
---
`invert`: two locals, `std::uint8_t* p = img.pixels;` and
`std::size_t n = img.size;`, then the same loop with `p` and `n`.
---
`threshold`: a range-`for` with a reference, so the assignment changes
the pixel in the vector:

```cpp
for (std::uint8_t& p : pixels) {
    p = p < level ? 0 : 255;
}
```

Or take `pixels.data()` and `pixels.size()` into locals, as in
`invert`.
