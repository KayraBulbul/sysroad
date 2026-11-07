---
title: std::optional and error values
minutes: 15
files: [parse.cpp]
run:
  args: ["4K", "12", "1.5M", "3G"]
deeper:
  - title: std::optional (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/optional
  - title: std::from_chars (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/from_chars
---
Lots of functions can fail in an ordinary way: parsing `"12abc"` as a
number, looking up a key that isn't there. C hands back a magic value
(`-1`, `NULL`, `0`), and every caller has to remember which one.
`std::optional<T>` says it in the type: either a `T`, or nothing.

```cpp
std::optional<int> parse_int(std::string_view s);

if (auto n = parse_int(arg)) {   // true when there's a value
    use(*n);                     // * gets the value out
} else {
    report_bad(arg);
}
int port = parse_int(text).value_or(8080);  // a default instead
```

Return `std::nullopt` (or just `{}`) for "nothing", and the value itself
for "something". It converts.

**`std::from_chars`** is the parser to use underneath. It reads a number
from a range of characters and tells you where it stopped and whether it
worked:

```cpp
int n = 0;
auto [end, ec] = std::from_chars(s.data(), s.data() + s.size(), n);
bool ok = ec == std::errc() && end == s.data() + s.size();  // all of it
```

It rejects overflow (`ec == std::errc::result_out_of_range`), and it
doesn't accept a leading `+` or spaces, which is what you want here.

**The spec**

- `parse_int`: the whole string must be an `int` (a leading `-` is fine).
- `parse_size`: a whole number, optionally followed by one suffix `K`, `M`
  or `G` (×1024, ×1024², ×1024³, upper or lower case). Fractions, other
  suffixes, empty strings and anything that overflows a 64-bit unsigned
  are `nullopt`.
- `main` prints each argument's size on its own line, or
  `size: invalid size 'X'` on stderr, and exits 1 if any argument was bad.

## Assignment

- [ ] Implement `parse_int` with `std::from_chars`
- [ ] Implement `parse_size`, including the overflow check
- [ ] Print each size or an error in `main`
