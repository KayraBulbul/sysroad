---
title: Moving the cursor
kind: capstone
program: kite
minutes: 25
files: [cursor.cpp]
run:
  args: [demo/poem.txt]
  interactive: true
---
kite's cursor is two numbers, `cx` and `cy`: a column and a row in the
**text**, not on the screen. The screen is just a window onto the
text, and next lesson it scrolls. `move_cursor` changes them for the
movement keys, and `draw_screen` puts the terminal's cursor there.

The rules are the ones you know from every editor:

```text
the fog comes|        Right at the end of a line goes to the next line's start;
on little cat feet.   Left at a line's start goes to the end of the one above.

it sits looking       Down from "on little cat feet." to this shorter line
                      snaps the cursor to its end.
```

The cursor can be on the text, or on the line **just past its end**,
where `cy == rows.size()`. That spare line is where you'll type to
add a line at the end of the file. It's empty, so the cursor is at
column 0 there, and nothing is to the right of it.

Page Up and Page Down move a screenful, `screen_rows` lines, which is
just pressing Up or Down that many times.

## Assignment

- [ ] Left, Right, Up and Down move one step, and the cursor never leaves the text (or the line past its end)
- [ ] Left at the start of a line wraps to the end of the line above; Right at the end of a line wraps to the start of the next one (or the line past the end)
- [ ] Home goes to column 0 and End to the end of the line
- [ ] Page Up and Page Down move `screen_rows` lines
- [ ] After any key, a cursor beyond the end of its line snaps to the end. Other keys change nothing
