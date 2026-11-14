---
title: Tabs, line endings and scrolling
kind: capstone
project: kite
program: kite
minutes: 45
run:
  args: [demo/sample.cxx]
  interactive: true
---
Open `demo/sample.cxx` in kite and two problems show: **tabs**, and a
file **bigger than the screen**.

A tab is one character that's drawn as up to eight columns: it moves
to the next **tab stop**, a multiple of 8. So each `Row` keeps a second
string. `chars` is the text as it is, and `render` is how it's drawn,
with tabs turned into spaces. The cursor needs both: `cx` is a
position in `chars`, and the screen needs the column in `render`,
which kite calls `rx`:

```text
chars:  a\tb          cx of 'b' is 2
render: a       b     rx of 'b' is 8
```

`render` is worked out from `chars` whenever `chars` changes, which for
now is when `open_file` reads it. Movement keeps working on `chars`, so
Right steps over a tab in one go.

For the size, the screen shows a window onto the text, starting at
text row `rowoff` and render column `coloff`. `draw_screen` draws the
rows from `rowoff` on, each `render` from column `coloff`, and puts the
cursor at `(rx - coloff, cy - rowoff)`. Before it draws, it **scrolls**:
it works out `rx`, then moves the window just far enough to keep the
cursor in view. If the cursor is above the window, the window's top
moves up to it. If it's below, the window moves down so the cursor is
on the bottom row. Left and right work the same way, in render columns.
That's why `draw_screen` takes a plain `Editor&`.

Files end lines with `\n` on Linux and the Mac, and `\r\n` on Windows.
`std::getline` stops at the `\n` and leaves the `\r` on the end, which
the terminal would act on: it sends the cursor back to the start of
the row, and the `\x1b[K` after it wipes the line out. kite shows both
kinds of file the same, so `open_file` drops a `\r` at the end of a
line.

## Contract

Nothing new. The tests open files with tabs, `\r\n` endings and more
lines than fit, move around them, and look at the screen.

## Assignment

- [ ] A file with `\r\n` line endings shows and moves the same as one with `\n`: the `\r` isn't part of the line
- [ ] A tab is drawn as spaces up to the next multiple of 8 (a tab at a tab stop takes all 8), and the cursor steps over it in one move
- [ ] The cursor is drawn where its character is on screen, after any tabs; on the line past the end, at column 0
- [ ] When the cursor would leave the screen, the view scrolls the least it can to keep it in sight, up, down, left or right, counting the columns tabs take
