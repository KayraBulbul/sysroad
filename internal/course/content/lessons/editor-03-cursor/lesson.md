---
title: Moving the cursor
kind: capstone
project: kite
program: kite
minutes: 35
contract:
  functions:
    - name: process_key
      sig: "bool(Editor&, int)"
run:
  args: [demo/poem.txt]
  interactive: true
---
kite's cursor is two numbers, `cx` and `cy`: a column and a row in the
**text**, not on the screen. The screen is just a window onto the
text, and next lesson it scrolls. The movement keys change them, and
`draw_screen` puts the terminal's cursor there.

Every key now goes through one function, `process_key`. It takes a key,
changes the editor, and returns `false` when it's time to quit, so
`main`'s loop becomes `if (!process_key(e, keys.next())) break;`. Keys
it has no use for do nothing. For now that's every key apart from
Ctrl-Q and these:

```text
the fog comes|        Right at the end of a line goes to the next line's start;
on little cat feet.   Left at a line's start goes to the end of the one above.

it sits looking       Down from "on little cat feet." to this shorter line
                      snaps the cursor to its end.
```

The cursor can be on the text, or on the line **just past its end**,
where `cy == rows.size()`. That spare line is where you'll type to
add a line at the end of the file. It's empty, so the cursor is at
column 0 there, and nothing is to the right of it. The status bar's
`3/9` follows the cursor, and shows `10/9` on that line.

Home goes to the start of the line and End to its end. Page Up and
Page Down move a screenful, as many lines as the text has rows on
screen, which is the same as pressing Up or Down that many times.

## Contract

```cpp
// Acts on one key. Returns false when it's time to quit.
bool process_key(Editor& e, int key);
```

The tests press keys the way a terminal sends them: they turn the
bytes into keys with your `decode_key`, hand each to `process_key`, and
draw a frame after each, as `main` does. Then they read the cursor off
the screen.

## Assignment

- [ ] Left, Right, Up and Down move one step, and the cursor never leaves the text (or the line past its end)
- [ ] Left at the start of a line wraps to the end of the line above; Right at the end of a line wraps to the start of the next one (or the line past the end)
- [ ] Home goes to column 0 and End to the end of the line
- [ ] Page Up and Page Down move as many lines as the text has rows
- [ ] After any key, a cursor beyond the end of its line snaps to the end. Ctrl-Q returns `false`, and keys kite doesn't use change nothing
