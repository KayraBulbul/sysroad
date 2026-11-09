`begin()` is `data_`, and `end()` is `data_ + size_`, one past the last
element.
---
In `insert`, `pos` stops being valid as soon as the vector grows, so
turn it into an index first: `std::size_t i = pos - begin();`. Then add
the value at the end and rotate it into place:

```cpp
emplace_back(std::move(value));
std::rotate(begin() + i, end() - 1, end());
return begin() + i;
```

`value` is taken by value, so it's a separate object even if the caller
passed one of your own elements.
---
Range `erase`: move the tail down over the gap, then destroy the
leftovers at the end:

```cpp
std::size_t i = first - begin(), n = last - first;
std::move(begin() + i + n, end(), begin() + i);
for (std::size_t k = 0; k < n; ++k) pop_back();
return begin() + i;
```

Single `erase(pos)` is `erase(pos, pos + 1)`.
---
`erase_if`:

```cpp
auto keep_end = std::remove_if(v.begin(), v.end(), pred);
std::size_t removed = v.end() - keep_end;
v.erase(keep_end, v.end());
return removed;
```
