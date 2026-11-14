One way: `Row` gets a `std::vector<Hl> hl;`, the editor a `bool highlight`. A
`refresh_row(e, y)` that remakes row `y`'s `render`, then its `hl`
(all `Normal` when `highlight` is off), replaces your calls that only
remade `render`. Setting `highlight` from the name is a loop over the
eight endings, comparing them with `filename.substr(filename.rfind('.'))`
(when there is a dot). Call it from `open_file`, after reading, and
when Save-as gives the file a name.
---
The walk, with `hl.assign(render.size(), Hl::Normal)` first:

```cpp
bool prev_sep = true;
char in_string = 0;
for (std::size_t i = 0; i < s.size(); ++i) {
    // in_string: String; a '\\' also colours s[i + 1] and skips it;
    //            the quote that opened it ends it; prev_sep = true
    // a quote:   in_string = it; String
    // a number:  Number; prev_sep = false
    // else:      prev_sep = is_separator(s[i])
}
```

A number continues while the previous character was one:
`Hl prev = i > 0 ? hl[i - 1] : Hl::Normal;` and the rule is
`(isdigit(c) && (prev_sep || prev == Hl::Number)) || ((c == '.' || isalnum(c)) && prev == Hl::Number)`.
Pass `std::isdigit` an `unsigned char`: a negative `char` (a UTF-8
byte) is undefined behaviour there.
---
Drawing a row in colour: keep the colour you last wrote, and for each
character on screen work out the one it needs. When they differ, write
the new one. The same goes for inverse (`\x1b[7m` on, `\x1b[27m` off),
so you can write both at once: `std::format("\x1b[{};{}m", colour, inverse ? "7" : "27")`.
After the row, `\x1b[m`.
---
A control character `c` (below 32, or 127) is drawn inverted, as
`c < 32 ? '@' + c : '?'`. For the match, the search can leave its
place and length in the editor (an `std::optional<Pos>` and an `int`),
set by its key callback and cleared when it ends. When drawing that
row, the inverted columns run from `cx_to_rx(row, match.col)` up to
`cx_to_rx(row, match.col + length)`.
