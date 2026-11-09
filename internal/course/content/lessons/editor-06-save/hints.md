`std::format` can cut a string for you: `{:.20}` prints at most 20
characters. The left side is
`std::format("{:.20} - {} lines{}", name, e.rows.size(), e.dirty ? " (modified)" : "")`.
Then fill the gap with `std::string(n, ' ')`.
---
Writing: open a `std::ofstream` on `e.filename + ".kite-save"` with
`std::ios::binary | std::ios::trunc`, write the text, and `close()` it
before checking `if (!out)`. Errors from flushing the last bytes only
show up on close.
---
`std::rename(from, to)` returns 0 on success. On failure, `errno` says
why, and `std::strerror(errno)` turns it into words like "No such file
or directory". Read `errno` straight away: other calls, like the
`std::remove` cleanup, can overwrite it.
