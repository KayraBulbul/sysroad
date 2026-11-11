`spaces(v)`: two comparisons and an OR. The comparisons give signed
char vectors, so cast the result:

```cpp
static u8x16 spaces(u8x16 v) {
    auto space = v == splat16(' ');
    auto control = (v >= splat16('\t')) & (v <= splat16('\r'));
    return reinterpret_cast<u8x16>(space | control);
}
```
---
The first byte of the chunk starts a word if it isn't whitespace and
`st.in_word` is false. Count it (and whether it's a newline) before
the loop. Add `n` to `st.counts.bytes` up front, and return early when
`n` is 0, because there's no first byte to look at.
---
The loop has the same shape as `count_byte`'s, with two lane counters:

```cpp
std::size_t i = 1;
while (i + 16 <= n) {
    u8x16 lines{}, starts{};
    for (int block = 0; block < 255 && i + 16 <= n; ++block, i += 16) {
        u8x16 v = load16(p + i);
        u8x16 before = load16(p + i - 1);
        lines -= reinterpret_cast<u8x16>(v == splat16('\n'));
        starts -= ~spaces(v) & spaces(before);
    }
    // add the 16 lanes of each into st.counts
}
// leftovers: compare p[i] with p[i - 1] the same way, one at a time
st.in_word = !is_space(static_cast<unsigned char>(p[n - 1]));
```
---
`count_fd`: a `std::vector<char> buf(256 * 1024)` and a loop. When
`read` returns -1 with `errno == EINTR`, a signal interrupted it
before it read anything: just try again.
