`decode_key` starts with the easy cases. If the first byte isn't ESC,
or it's the only byte, return
`{static_cast<unsigned char>(bytes[0]), 1}`. If the second byte is `[`,
it's a sequence to parse. If it's `O` and there's a third byte, look at
that one. Anything else is `{27, 1}`.
---
For `ESC [`, skip the parameter bytes, then check for a final byte:

```cpp
std::size_t i = 2;
while (i < bytes.size() && bytes[i] >= 0x30 && bytes[i] <= 0x3f) ++i;
if (i == bytes.size() || bytes[i] < 0x40 || bytes[i] > 0x7e) return {27, 1};
std::string_view params = bytes.substr(2, i - 2);
char final = bytes[i];
std::size_t used = i + 1;
```

Then `if (params.empty() && final == 'A') return {kArrowUp, used};`,
`if (final == '~' && params == "3") return {kDelete, used};`, and so
on, ending with `return {kUnknown, used};`.
---
A key reader is a small class with the fd and a `std::string` of
pending bytes:

```cpp
int KeyReader::next() {
    if (pending_.empty()) fill(true);        // read until something comes
    auto [key, used] = decode_key(pending_);
    while (key == 27 && could_be_sequence(pending_) && fill(false)) {
        std::tie(key, used) = decode_key(pending_);   // more of it arrived
    }
    pending_.erase(0, used);
    return key;
}
```

where a sequence is still possible for a lone ESC, or an unfinished
`ESC [` or `ESC O` that isn't absurdly long:

```cpp
bool could_be_sequence(const std::string& b) {
    return b.size() == 1 || (b.size() < 8 && (b[1] == '[' || (b[1] == 'O' && b.size() == 2)));
}
```

`fill(wait)` calls `read` into a buffer and appends what came. With
`wait` it loops until `read` returns something; without, it tries once,
which waits at most a tenth of a second, and returns whether anything
came.
---
The names are a `switch` on the key for the special ones, then:

```cpp
if (key >= 1 && key <= 26) return std::string("Ctrl-") + static_cast<char>('A' + key - 1);
if (key > 32 && key < 127) return std::string(1, static_cast<char>(key));
return "byte " + std::to_string(key);
```

Put Tab and Enter in the `switch`, before this: they're Ctrl-I and
Ctrl-M too.
