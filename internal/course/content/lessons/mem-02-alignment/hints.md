`align_up(5, 4)` is 8, `align_up(8, 4)` is 8, `align_up(0, 8)` is 0.
With the mask trick: `(n + align - 1) & ~(align - 1)`. Or without it:
`(n + align - 1) / align * align`.
---
Keep a running `offset`, the end of the last field. For each field:

```cpp
std::size_t at = align_up(offset, f.align);
out.fields.push_back({f.name, at, f.size, at - offset});
offset = at + f.size;
out.align = std::max(out.align, f.align);
```
---
After the loop, `out.size = align_up(offset, out.align)` and the tail
padding is `out.size - offset`. Start `out.align` at 1, not 0.
