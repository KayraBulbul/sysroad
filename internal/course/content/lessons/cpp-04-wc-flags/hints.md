Loop over the arguments, and skip any that aren't flags:

```cpp
for (std::string_view arg : args) {
    if (!arg.starts_with('-') || arg.size() == 1) continue;
    for (char ch : arg.substr(1)) { /* ... */ }
}
```
---
A `switch (ch)` with `case 'l': o.lines = true; break;` and so on. In the
`default:` case, set `o.bad = ch;` and `return o;` straight away.
---
After the loop:
`if (!o.lines && !o.words && !o.bytes) o.lines = o.words = o.bytes = true;`
---
Build the result with a `std::vector<std::size_t>` of the chosen counts,
then join them with `" "`, formatting each one with `std::format("{:>7}", n)`.
