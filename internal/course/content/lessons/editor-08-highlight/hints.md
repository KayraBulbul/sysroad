Start with `row.hl.assign(render.size(), Hl::Normal)`, then a `while`
loop over `i` with the rules as `if` blocks, each ending in `continue`.
Keep `prev_sep` (start it at true), `char in_string = 0`, and a `bool
preproc`. A helper saves typing:

```cpp
bool at(const std::string& s, std::size_t i, std::string_view w) {
    return s.compare(i, w.size(), w) == 0;
}
```
---
A number continues while the previous character was a number:
`Hl prev = i > 0 ? row.hl[i - 1] : Hl::Normal`. The rule is
`(isdigit(c) && (prev_sep || prev == Hl::Number)) || ((c == '.' || isalnum(c)) && prev == Hl::Number)`.
Pass `std::isdigit` an `unsigned char`: a negative `char` (a UTF-8
byte) is undefined behaviour there.
---
Keywords: when `prev_sep` is true, try each word in `kKeywords`, then
`kTypes`. It matches if the text at `i` starts with it and the
character after it is the end of the line or a separator. Fill `len`
entries with `std::fill`, move `i` past it, and set `prev_sep` to false.

For `#`: it starts a preprocessor line if it's the first thing on the
line apart from spaces and tabs, `s.find_first_not_of(" \t") == i`.
