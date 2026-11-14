The editor has three more things to remember: the file's name, the
message, and when it was set (a `std::time_t`). A helper sets the last two together:

```cpp
void set_status(Editor& e, std::string msg) {
    e.status_msg = std::move(msg);
    e.status_time = std::time(nullptr);
}
```

In `set_window_size`, `e.screen_rows = std::max(rows - 2, 0);`. Every
text row now ends with `\r\n`, since the bars come after it.
---
`std::format` can cut a string for you: `{:.20}` prints at most 20
characters. The left side is
`std::format("{:.20} - {} lines", name, e.rows.size())`, and the right
is `std::format("{}/{}", e.cy + 1, e.rows.size())`.
---
Fitting the bar to `width`:

```cpp
if (left.size() > width) left.resize(width);
if (left.size() + right.size() <= width)
    return left + std::string(width - left.size() - right.size(), ' ') + right;
return left + std::string(width - left.size(), ' ');
```

Draw it as `"\x1b[7m" + bar + "\x1b[m\r\n"`, then `\x1b[K` and the
message, if `std::time(nullptr) - e.status_time < 5`.
---
In `main`, set the help message before `open_file`, so a new file's
`New file` is what shows.
