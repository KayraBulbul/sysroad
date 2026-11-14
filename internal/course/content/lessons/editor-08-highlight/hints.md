Start with the state the walk keeps: `prev_sep` (start it at true),
`char in_string = 0`, `bool preproc`, and now the comment state, passed
in and returned: `bool highlight_row(Row& row, bool in_comment)`. A
helper saves typing:

```cpp
bool at(const std::string& s, std::size_t i, std::string_view w) {
    return s.compare(i, w.size(), w) == 0;
}
```

Order the rules as the lesson lists them, each an `if` ending in
`continue`. The `//` and `/*` rules check that no string (and, for
`//`, no comment) is open; the "inside a comment" rule needs no such
check, since a string can't open inside a comment.
---
Keywords: when `prev_sep` is true, try each word in your keyword list,
then the types. A word matches if the text at `i` starts with it and
the character after it is the end of the line or a separator. Fill
`len` entries with `std::fill`, move `i` past it, and set `prev_sep`
to false. A `std::array<std::string_view, 37>` holds the keywords
without copying any strings.
---
For `#`: it starts a preprocessor line if it's the first thing on the
line apart from spaces and tabs, `s.find_first_not_of(" \t") == i`.
Then everything that isn't a string, number or comment is `Preproc`.
---
Each row remembers three more things: `open_comment` (it ends inside a
comment), `hl_in` (it was coloured starting inside one) and `hl_done`
(its colours are up to date). Colouring from row `y` on:

```cpp
bool in = y > 0 && e.rows[y - 1].open_comment;
for (std::size_t r = y; r < e.rows.size(); ++r) {
    Row& row = e.rows[r];
    if (r > y && row.hl_done && row.hl_in == in) break;   // still right
    row.hl_in = in;
    row.hl_done = true;
    row.open_comment = highlight_row(row, in);
    in = row.open_comment;
}
```

After an edit, set the row's `hl_done` to false and run this from it.
A new row starts with `hl_done` false, so the loop never stops on it.
