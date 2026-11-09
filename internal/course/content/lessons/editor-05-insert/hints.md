`std::string` has what you need: `insert(pos, 1, c)`,
`erase(pos, 1)`, `substr(pos)` and `resize(n)`. For the rows,
`e.rows.insert(e.rows.begin() + i, row)` and
`e.rows.erase(e.rows.begin() + i)`.
---
`insert_newline`: the text after the cursor becomes a new row, and the
current row is cut short:

```cpp
Row rest{row.chars.substr(e.cx), ""};
row.chars.resize(e.cx);
```

Update both renders, insert `rest` after row `cy`, then `++cy` and
`cx = 0`. Insert `rest` last: inserting into `e.rows` can move every
row, which would leave a `Row&` dangling.
---
`delete_char` at column 0: the cursor goes to the end of the line
above, *before* it gets longer, so remember its old length first. Then
append, update its render, and erase the current row.
