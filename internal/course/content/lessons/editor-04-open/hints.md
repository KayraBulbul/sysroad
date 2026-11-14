In the lesson's design, `Row` gets `std::string render;`, and `Editor`
gets `rx`, `rowoff` and `coloff`, all starting at 0. In `open_file`, after `std::getline`,
check for a `\r` left at the end with `line.back()`, then fill in
`render` before you push the row.
---
For a tab, add one space, then keep adding spaces while
`render.size() % 8 != 0`. Working out `rx` is the same sum without
building a string:

```cpp
int cx_to_rx(const Row& row, int cx) {
    int rx = 0;
    for (int j = 0; j < cx; ++j) {
        if (row.chars[j] == '\t') rx += 7 - (rx % 8);
        ++rx;
    }
    return rx;
}
```
---
Scrolling, with the rows first:

```cpp
if (e.cy < e.rowoff) e.rowoff = e.cy;
if (e.cy >= e.rowoff + e.screen_rows) e.rowoff = e.cy - e.screen_rows + 1;
```

The columns are the same with `rx`, `coloff` and `screen_cols`. Work
out `rx` first: `cx_to_rx(e.rows[e.cy], e.cx)` on a line of text, 0 on
the line past the end.
---
Drawing from the offsets: the text row on screen row `y` is
`y + rowoff`, and its part on screen is
`render.substr(coloff, cols)`, but only when `coloff < render.size()`:
`substr` throws if it starts past the end. The cursor goes to
`std::format("\x1b[{};{}H", e.cy - e.rowoff + 1, e.rx - e.coloff + 1)`.
