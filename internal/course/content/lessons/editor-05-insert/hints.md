A function per operation keeps `process_key` short: `case kEnter:
insert_newline(e); break;` and so on, with the typing keys in the
`default:` case:

```cpp
if (key == kTab || (key >= 32 && key < 256 && key != kBackspace))
    insert_char(e, static_cast<char>(key));
```
---
`std::string` has what you need: `insert(pos, 1, c)`, `erase(pos, 1)`,
`substr(pos)` and `resize(n)`. For the rows,
`e.rows.insert(e.rows.begin() + i, row)` and
`e.rows.erase(e.rows.begin() + i)`. After changing a row's `chars`,
call your function that rebuilds `render`.
---
Enter: the text after the cursor becomes a new row, and the current
row is cut short:

```cpp
Row rest{row.chars.substr(e.cx), ""};
row.chars.resize(e.cx);
```

Update both renders, insert `rest` after row `cy`, then `++cy` and
`cx = 0`. Insert `rest` last: inserting into `e.rows` can move every
row, which would leave a `Row&` dangling.
---
Backspace at column 0: the cursor goes to the end of the line above,
*before* it gets longer, so take its old length first. Then append,
update its render, and erase the current row. Delete at a line's end is
the mirror: append the next row's `chars` to this one and erase the
next row, while `cy + 1 < rows.size()`.
