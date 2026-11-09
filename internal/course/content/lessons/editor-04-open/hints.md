`std::getline` already drops the `\n`, and its loop handles a last line
with no newline. Check for a `\r` left at the end with `line.back()`.
---
For a tab, add one space, then keep adding spaces while
`render.size() % kTabStop != 0`. `cx_to_rx` does the same sum without
building a string: at a tab, `rx += (kTabStop - 1) - (rx % kTabStop)`,
then `rx++` for every character.
---
`scroll`, with the rows first:

```cpp
if (e.cy < e.rowoff) e.rowoff = e.cy;
if (e.cy >= e.rowoff + e.screen_rows) e.rowoff = e.cy - e.screen_rows + 1;
```

The columns are the same with `rx`, `coloff` and `screen_cols`.
