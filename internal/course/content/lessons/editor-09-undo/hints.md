`undo` is a handful of lines:

```cpp
Edit edit = std::move(h.undo.back());
h.undo.pop_back();
if (edit.kind == Edit::Kind::Insert) erase_text(e, edit.at, edit.text.size());
else insert_text(e, edit.at, edit.text);
```

then the cursor, `dirty`, `h.redo.push_back(std::move(edit))` and
`h.sealed = true`. In `redo`, `insert_text` returns where the text
ends, which is where the cursor goes.
---
For `record`, write `joins(last, edit)` first. Both must be the same
kind, on the same row, with no `'\n'` in either text. Then:

- typing: `edit.at.col == last.at.col + last.text.size()`, and not a
  word after a space: `last.text.back() == ' ' && edit.text.front() != ' '`
- Backspace: `edit.at.col + edit.text.size() == last.at.col`
- Delete: `edit.at.col == last.at.col`
---
Joining Backspace moves the step's start left: `last.at = edit.at` and
`last.text = edit.text + last.text`. Keep `last.cursor` as it is: undo
should put the cursor where it was before the *first* Backspace.
Typing and Delete just append: `last.text += edit.text`.
