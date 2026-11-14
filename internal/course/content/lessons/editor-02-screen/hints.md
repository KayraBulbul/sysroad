Build the frame with `+=`. A loop over `y` from 0 to `screen_rows`
decides what each row is: `y < rows.size()` is a line of text
(`chars.substr(0, cols)`), and the rest are `~`, apart from the welcome
row. After each row, `\x1b[K`, and `\r\n` if `y < screen_rows - 1`.
---
For the welcome, cut first: `kWelcome.substr(0, cols)`, with
`constexpr std::string_view kWelcome = "kite -- a tiny text editor";`.
The padding is `(cols - welcome.size()) / 2`. If it's more than zero,
write `~`, then `padding - 1` spaces (`out.append(n, ' ')`), then the
message.
---
`std::format("\x1b[{};{}H", e.cy + 1, e.cx + 1)` moves the cursor.
Terminal rows and columns count from 1, and kite's from 0. The frame
starts with `\x1b[?25l\x1b[H` and ends with that move and `\x1b[?25h`.
---
The plumbing, in your terminal file:

```cpp
struct WindowSize { int rows = 24; int cols = 80; };

WindowSize window_size(int fd) {
    winsize ws{};
    if (ioctl(fd, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) return {};  // 24 x 80
    return {ws.ws_row, ws.ws_col};
}

void write_all(std::string_view s) {
    while (!s.empty()) {
        ssize_t n = write(STDOUT_FILENO, s.data(), s.size());
        if (n == -1) {
            if (errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category(), "write");
        }
        s.remove_prefix(static_cast<std::size_t>(n));
    }
}
```

The alternate-screen class writes `\x1b[?1049h` in its constructor and
`\x1b[?1049l` in its destructor. Destructors mustn't throw, so wrap
that one in `try { ... } catch (...) {}`.
---
`main`, in order:

```cpp
Editor e;
if (argc > 1) open_file(e, argv[1]);
try {
    RawMode raw(STDIN_FILENO);
    AltScreen alt;
    KeyReader keys(STDIN_FILENO);
    while (true) {
        WindowSize ws = window_size(STDOUT_FILENO);
        set_window_size(e, ws.rows, ws.cols);
        write_all(draw_screen(e));
        if (keys.next() == ctrl_key('q')) break;
    }
} catch (const std::exception& ex) { /* print, return 1 */ }
return 0;
```
