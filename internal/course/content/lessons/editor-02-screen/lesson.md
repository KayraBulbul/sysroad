---
title: Drawing the screen with escape codes
base: editor-01-raw
kind: capstone
program: kite
minutes: 30
files: [render.cpp]
run:
  interactive: true
deeper:
  - title: "VT100 escape sequences (vt100.net)"
    url: https://vt100.net/docs/vt100-ug/chapter3.html
  - title: "ANSI escape code (Wikipedia)"
    url: https://en.wikipedia.org/wiki/ANSI_escape_code
---
A terminal draws whatever bytes it's sent, and some byte sequences are
commands instead of text. They start with ESC `[`, like the keys did,
and they move the cursor, clear lines and change colours:

```text
\x1b[H          move the cursor to the top left (row 1, column 1)
\x1b[12;5H      move it to row 12, column 5 (counting from 1)
\x1b[K          clear from the cursor to the end of the line
\x1b[?25l       hide the cursor; \x1b[?25h shows it again
```

kite redraws the whole screen after every key. `draw_screen` builds
the redraw as one string, and `main` writes it in a single `write`.
Lots of small writes would make the terminal show half-drawn frames,
which flicker.

Each screen row is a line of the text, or a `~` past the end of the
file, the way vi shows it. Each row ends with `\x1b[K` to wipe what the
last frame left there, then `\r\n`, except the last: a newline on the
bottom row would scroll the whole screen up by one. The cursor is hidden
while drawing, so it doesn't flash across the screen, then moved to
`(cx, cy)` and shown.

A file with no lines shows `kWelcome` a third of the way down,
centred:

```text
~
~      kite -- a tiny text editor
~
```

## Assignment

- [ ] Start with `\x1b[?25l\x1b[H`, and end with the cursor at `cy + 1`, `cx + 1` (`\x1b[row;colH`) then `\x1b[?25h`
- [ ] Draw each of the `screen_rows` rows: the text row's `render`, cut at `screen_cols`, or `~` past the end of the text
- [ ] Each row ends with `\x1b[K`, and every row but the last with `\r\n`
- [ ] With no text at all, the row at `screen_rows / 3` is `kWelcome`, cut at `screen_cols`, centred with the spare columns halved (rounding down) to the left, and a `~` as the first of them
