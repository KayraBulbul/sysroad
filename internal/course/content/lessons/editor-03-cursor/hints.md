A `switch (key)` with a case per key, then the snapping after it. A
small helper saves a lot of repetition:

```cpp
// The length of row y, or 0 for the line past the end.
int row_len(const Editor& e, int y) {
    if (static_cast<std::size_t>(y) >= e.rows.size()) return 0;
    return static_cast<int>(e.rows[y].chars.size());
}
```
---
Down is allowed while `cy < rows.size()`, so it can reach the line past
the end but not beyond. Right on the line past the end does nothing,
since there's no line after it to wrap to.
---
Page Up and Page Down can call `move_cursor` itself, `screen_rows`
times, with `kArrowUp` or `kArrowDown`.
