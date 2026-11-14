If your prompt is last lesson's struct, give it a second callback,
`std::function<void(Editor&, const std::string& answer, int key)> on_key;`.
In your prompt's key handling, after the answer has taken the key (and
only for keys that don't end the prompt):
`if (p.on_key) p.on_key(e, p.answer, key);`. A `Pos { int row, col; }`
struct is handy for places in the text.
---
`std::string` does the work within a line: `chars.find(query, col)` is
the first match starting at or after `col`, and
`chars.rfind(query, col)` is the last one starting at or before `col`.
Both return `std::string::npos` when there's none.
---
A `find(rows, query, from, dir)` of your own, forwards, is two loops.
The first runs from `from.row` to the last row, searching from
`from.col` on `from`'s own row and from 0 on the rest. The second, the
wrap, runs from row 0 and takes the first match it sees. Backwards is
the mirror, with `rfind`. Watch the edges: `find` with a position past
the end of the string is fine (it returns `npos`), but a negative
column cast to `std::size_t` becomes huge, and for `rfind` that means
"from the end". Check for `col < 0` first and skip that row.
---
The search's own state, shared by both callbacks:

```cpp
struct Search { int cx, cy, rowoff, coloff; std::optional<Pos> match; };
auto s = std::make_shared<Search>(Search{e.cx, e.cy, e.rowoff, e.coloff, std::nullopt});
p.on_key = [s](Editor& e, const std::string& query, int key) {
    Pos from{s->cy, s->cx};
    int dir = 1;
    if (s->match && (key == kArrowRight || key == kArrowDown)) from = {s->match->row, s->match->col + 1};
    // ... Left and Up: col - 1, dir = -1
    s->match = find(e.rows, query, from, dir);
    // a match: move the cursor there; none: put cx, cy, rowoff, coloff back
};
p.done = [s](Editor& e, std::optional<std::string> answer) { /* Escape: go back; no match: Not found */ };
```
