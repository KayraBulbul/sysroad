A small helper makes everything easier:

```cpp
static std::string_view strip(std::string_view s) {
    auto b = s.find_first_not_of(" \t\r");
    if (b == std::string_view::npos) return {};
    auto e = s.find_last_not_of(" \t\r");
    return s.substr(b, e - b + 1);
}
```
---
Read with `std::getline` and count lines as you go. Split at the *first*
`=`: `auto eq = text.find('=');`. The key is `strip(text.substr(0, eq))`
and the value is `strip(text.substr(eq + 1))`.
---
For duplicates, `if (cfg.contains(key)) throw ConfigError(n, "duplicate key '" + key + "'");`.
`std::map::contains` is C++20.
