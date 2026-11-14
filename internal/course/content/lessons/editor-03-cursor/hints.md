`process_key` is a `switch (key)`: `case ctrl_key('q'): return false;`,
then the movement keys, all sent to one `move_cursor(e, key)` helper of
your own. `return true;` at the end covers every other key.
---
In `move_cursor`, a case per key, then the snapping after the
`switch`. A small helper saves a lot of repetition:

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
times, with the Up or Down key:

```cpp
for (int i = 0; i < e.screen_rows; ++i)
    move_cursor(e, key == kPageUp ? kArrowUp : kArrowDown);
```
