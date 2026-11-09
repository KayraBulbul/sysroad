`std::string` does the work within a line: `chars.find(query, col)` is
the first match starting at or after `col`, and
`chars.rfind(query, col)` is the last one starting at or before `col`.
Both return `std::string::npos` when there's none.
---
Forwards is two loops. The first runs from `from.row` to the last row,
searching from `from.col` on `from`'s own row and from 0 on the rest.
The second, the wrap, runs from row 0 and takes the first match it
sees. Matches at or after `from` were already found by the first loop,
so the wrap only turns up ones before it.
---
Watch the edges: `find` with a position past the end of the string is
fine (it returns `npos`), but a negative column cast to `std::size_t`
becomes huge, and for `rfind` that means "from the end". Check for
`col < 0` first and skip that row.
