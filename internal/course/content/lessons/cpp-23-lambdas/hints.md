The comparison for `sort_by_size`, ties broken by name:

```cpp
[](const Entry& a, const Entry& b) {
    if (a.size != b.size) return a.size > b.size;
    return a.name < b.name;
}
```
It must be a strict "comes before": never true for two equal entries.
---
`std::accumulate(v.begin(), v.end(), std::uint64_t{0}, [](std::uint64_t sum, const Entry& e) { return sum + e.size; })`.
Write the start value as `std::uint64_t{0}`. A plain `0` is an `int`, and
the sum would be done in `int` and overflow.
---
`std::max_element(v.begin(), v.end(), cmp)` returns an iterator, which is
`v.end()` when `v` is empty. `cmp(a, b)` means "a is less than b", so
compare sizes with `<`. On ties it returns the first of the biggest.
