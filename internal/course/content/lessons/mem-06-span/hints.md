Every read goes through `bytes`, so the end check lives in one place:

```cpp
std::span<const unsigned char> ByteReader::bytes(std::size_t n) {
    if (n > remaining()) {
        ok_ = false;
        return {};
    }
    auto out = data_.subspan(pos_, n);
    pos_ += n;
    return out;
}
```

`n > remaining()` can't overflow. `pos_ + n > data_.size()` can, when
`n` is close to `SIZE_MAX`, and then a huge `n` looks small.
---
The number readers use `endian.hpp` from lesson 2.1.4 on the span's
`data()`:

```cpp
std::uint32_t ByteReader::le32() {
    auto b = bytes(4);
    return b.empty() ? 0 : load_le32(b.data());
}
```
---
`skip(n)` can just call `bytes(n)` and ignore the result. `seek(pos)`
sets `pos_` if `pos <= data_.size()`, and otherwise clears `ok_`.
Seeking to exactly `size()` is fine: that's the end, where every
further read fails.
