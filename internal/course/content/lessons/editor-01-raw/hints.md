`raw_attrs` is four lines of bit masks and two of `c_cc`. To clear
flags, `&=` with the complement: `t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);`.
On the Mac the flag fields are `unsigned long`, so wrap the mask in
`static_cast<tcflag_t>(...)` to keep the compiler quiet.
---
`decode_key`: if the first byte isn't ESC, or it's the only byte,
return it, as `static_cast<unsigned char>(bytes[0])` so bytes above 127
don't come out negative. If the next byte is `[`, skip the parameter
bytes (`'0'` to `'?'`), and check there's a final byte from `'@'` to
`'~'`. No final byte means the sequence isn't all here: that's `kEsc`.
---
With the parameters as a `string_view` and the final byte in hand:

```cpp
if (params.empty() && final == 'A') return {kArrowUp, used};
if (final == '~' && params == "3") return {kDelete, used};
// ... and kUnknown for anything else
```

`used` is everything up to and including the final byte.
