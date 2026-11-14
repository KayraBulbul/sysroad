The data:

```cpp
struct Edit {
    enum class Kind { Insert, Erase };
    Kind kind;
    Pos at;
    std::string text;   // may hold line breaks
    Pos cursor;         // where the cursor was before
};
struct History { std::vector<Edit> undo, redo; };
```

With a `History history;` in the editor, a `record(h, edit)` that
empties `h.redo` and pushes the edit onto `h.undo` is all the
bookkeeping.
---
`insert_text`: cut the row at `at` into the part before and the tail
after; append the text up to each `'\n'` to the current row, and at
each `'\n'` insert a new empty row after it and carry on there; then
put the tail back on the last row. The end position is that row and
the length before the tail. If `at` is the line past the end, add an
empty row first.
---
`erase_text`: from `at`, erase what's left of the row (or `n`, if
less); while there's still some of `n` to go and a next row, join the
next row on (that's the line break, one character) and carry on.
Remake `render` and colours for the rows you changed: with last
lesson's colouring, starting from the first changed row covers the
rest.
---
`undo` is a handful of lines:

```cpp
Edit edit = std::move(h.undo.back());
h.undo.pop_back();
if (edit.kind == Edit::Kind::Insert) erase_text(e, edit.at, edit.text.size());
else insert_text(e, edit.at, edit.text);
```

then the cursor, `dirty`, and `h.redo.push_back(std::move(edit))`. In
`redo`, `insert_text` returns where the text ends, which is where the
cursor goes, and the edit goes straight back on `h.undo`, not through
`record`, which would empty redo.
