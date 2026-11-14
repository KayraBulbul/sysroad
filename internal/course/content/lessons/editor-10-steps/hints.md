A flag in your history does it, `bool sealed = false;`. In `process_key`, set it after
the movement keys and Ctrl-F; `undo` and `redo` set it at their ends;
`record` clears it at its end.
---
Write `joins(last, edit)` first. Both must be the same kind, on the
same row, with no `'\n'` in either text. Then:

- typing: `edit.at.col == last.at.col + last.text.size()`, and not a
  word after a space: `last.text.back() == ' ' && edit.text.front() != ' '`
- Backspace: `edit.at.col + edit.text.size() == last.at.col`
- Delete: `edit.at.col == last.at.col`
---
`record` with joining:

```cpp
h.redo.clear();
if (!h.sealed && !h.undo.empty() && joins(h.undo.back(), edit)) {
    Edit& last = h.undo.back();
    if (edit.kind == Edit::Kind::Insert) last.text += edit.text;
    else if (edit.at.col < last.at.col) { last.at = edit.at; last.text = edit.text + last.text; }  // Backspace
    else last.text += edit.text;                                                                 // Delete
} else {
    h.undo.push_back(std::move(edit));
}
h.sealed = false;
```

Keep `last.cursor` as it is in every case.
