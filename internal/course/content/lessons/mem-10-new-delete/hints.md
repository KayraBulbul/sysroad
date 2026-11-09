`new int[n]` leaves the ints uninitialised. Fill them with a loop, or
use `std::fill(p, p + n, value)` from `<algorithm>`.
---
`grow` in four steps: allocate `new int[new_n]()` (the `()` zeroes it),
copy `std::min(old_n, new_n)` ints across, `delete[] old`, return the
new pointer. `delete[] nullptr` is allowed and does nothing, so
`grow(nullptr, 0, 4)` needs no special case.
---
Measure inside the function with an `AllocScope`:

```cpp
AllocScope scope;
{
    std::vector<int> v;
    if (reserve_first) v.reserve(n);
    for (std::size_t i = 0; i < n; ++i) v.push_back(int(i));
}
return scope.news();
```
