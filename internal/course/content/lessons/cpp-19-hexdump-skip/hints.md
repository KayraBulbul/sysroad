Skipping at the top of `feed`: take
`n = std::min<std::uint64_t>(to_skip_, bytes.size())`, then subtract it
from `to_skip_`, add it to `offset_`, set `consumed_ = true`, and
`bytes.remove_prefix(n)`. A 100-byte skip fed in 10-byte chunks takes ten
calls.
---
Length: if `o_.has_length`, cut `bytes` down to `o_.length` and subtract
what you kept from `o_.length`. Once it reaches 0, every later chunk is
cut to nothing.
---
Squeezing in `line`:

```cpp
if (!o_.no_squeeze && bytes.size() == 16 && bytes == prev_) {
    if (!squeezing_) { out_ << "*\n"; squeezing_ = true; }
} else {
    out_ << hex_line(offset_, bytes);
    squeezing_ = false;
}
if (bytes.size() == 16) prev_ = bytes;
offset_ += bytes.size();
```
