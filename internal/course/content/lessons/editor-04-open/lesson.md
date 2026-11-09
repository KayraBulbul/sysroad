---
title: Opening files and scrolling
kind: capstone
program: kite
minutes: 35
files: [view.cpp]
run:
  args: [demo/sample.cpp]
  interactive: true
---
Now kite opens a real file, and a real file brings two problems:
**tabs**, and being **bigger than the screen**.

A tab is one character that's drawn as up to eight columns: it moves
to the next **tab stop**, a multiple of 8. So each `Row` keeps two
strings. `chars` is the text as it is, and `render` is how it's drawn,
with tabs turned into spaces. The cursor needs both: `cx` is a
position in `chars`, and the screen needs the column in `render`,
which kite calls `rx`:

```text
chars:  a\tb          cx of 'b' is 2
render: a       b     rx of 'b' is 8
```

For the size, the screen shows a window of the text, starting at row
`rowoff` and render column `coloff`. Before every frame, `scroll`
moves the window just far enough to keep the cursor in view. If the
cursor is above the window, the window's top moves up to it. If it's
below, the window moves down so the cursor is on the bottom row. Left
and right work the same way, in render columns. The course's
`draw_screen` now draws from `rowoff` and `coloff`.

Files end lines with `\n` on Linux and the Mac, and `\r\n` on Windows.
kite shows both the same, so `read_rows` drops a `\r` at the end of a
line.

## Assignment

- [ ] `read_rows` returns one `Row` per line, without the `\n` or `\r\n`, and with `render` filled in. A last line with no newline still counts; an empty file has no rows
- [ ] `update_render` copies `chars` into `render`, turning each tab into spaces up to the next multiple of `kTabStop` (a tab at a tab stop takes all 8)
- [ ] `cx_to_rx` gives the render column of `chars` position `cx`
- [ ] `scroll` sets `e.rx` (0 on the line past the end), then moves `rowoff` and `coloff` the least it can so the cursor is on screen
