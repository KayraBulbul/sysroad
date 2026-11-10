The constructor needs `(size + 63) / 64` words, all zero. That's
"divide by 64, rounding up".

Write the bounds check once and call it from `test`, `set` and `flip`:

```cpp
static void check(std::size_t i, std::size_t size) {
    if (i >= size) {
        throw std::out_of_range("Bitset: bit " + std::to_string(i) + " of " + std::to_string(size));
    }
}
```

The mask for bit `i` is `std::uint64_t{1} << (i % 64)`. It has to be a
64-bit 1. A plain `1` is an `int`, the bug from lesson 4.1.4.
---
To put the unused bits back to 0, write one helper and call it at the
end of `set_all` and `operator~`:

```cpp
static void trim(std::vector<std::uint64_t>& words, std::size_t size) {
    if (size % 64 != 0) {
        words.back() &= (std::uint64_t{1} << (size % 64)) - 1;
    }
}
```

When `size` is a multiple of 64 the last word is full and there's
nothing to trim. That also covers a size of 0, which has no words.
---
`find_next(i)` is "the first set bit at index `i + 1` or later".
Write a helper that searches from a starting index, and `find_first`
is the same helper starting at 0:

```cpp
std::size_t w = from / 64;
std::uint64_t word = words[w] & (~std::uint64_t{0} << (from % 64));  // drop the bits below `from`
while (true) {
    if (word != 0) return w * 64 + std::countr_zero(word);
    if (++w == words.size()) return size;
    word = words[w];
}
```

Check `from >= size` first and return `size`. Because the unused bits
are 0, a hit is always a real bit.
---
The operators check `size_ != other.size_` first and throw
`std::invalid_argument`. Then it's one line per word:
`words_[w] &= other.words_[w];`. They return `*this`.

`operator~` copies `*this`, flips every word of the copy with `~`,
trims it, and returns it.
