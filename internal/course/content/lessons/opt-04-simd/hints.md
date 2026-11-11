`is_ascii`:

```cpp
unsigned char seen = 0;
for (std::size_t i = 0; i < n; ++i) {
    seen |= static_cast<unsigned char>(s[i]);
}
return seen < 128;
```

Rerun the `-Rpass` command from the lesson: the remark should now say
`vectorized loop`.
---
The comparison gives a vector of signed chars, so cast it before
subtracting: `counts -= reinterpret_cast<u8x16>(load16(s + i) == needle);`
where `needle` is `splat16(static_cast<std::uint8_t>(c))`, made once
before the loop.
---
Two loops, one inside the other:

```cpp
while (i + 16 <= n) {
    u8x16 counts{};
    for (int block = 0; block < 255 && i + 16 <= n; ++block, i += 16) {
        // compare and subtract
    }
    for (int lane = 0; lane < 16; ++lane) {
        total += counts[lane];
    }
}
// then the last n % 16 bytes, one by one
```
