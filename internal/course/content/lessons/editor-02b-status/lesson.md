---
title: The status bar and messages
kind: capstone
project: kite
program: kite
minutes: 35
retires:
  - lesson: editor-02-screen
    cases: ["the tildes run all the way to the bottom row", "the bottom row is the last line that fits"]
run:
  args: [demo/poem.txt]
  interactive: true
---
kite gets two bars under the text. The **status bar**, in inverted
colours, says what you're editing. The **message bar** under it shows
messages, like the keys to press, for five seconds:

```text
~
poem.txt - 9 lines                                      1/9
Ctrl-Q quit
```

The bars take the window's last two rows, so the text gets the rest:
`set_window_size` keeps the number of rows for **text**, two fewer than
the window has. Everything that used the window's height now uses the
text's: the tildes stop above the bars, and the welcome sits a third of
the way down the text's rows. Two of the last lesson's tests checked
the window's bottom row, which is a bar now, so they retire.

The status bar is exactly as wide as the window. On the left, the
file's name (or `[No Name]` with none), cut to 20 characters, then
` - N lines`. On the right, the cursor's line, counted from 1, and the
number of lines: `3/9`. Spaces fill the gap between them, so the
inverted bar runs edge to edge. When the window is too narrow for both
sides, the right side goes, and then the left is cut to fit.

```text
\x1b[7m      inverted colours: the text's colour behind, the background's in front
\x1b[m       back to normal
```

The message bar is the last row: the message, cut to the window's
width, if it was set less than five seconds ago, else nothing. A
message is a string and the time it was set (`std::time(nullptr)`
counts seconds). kite shows its keys when it starts (`Ctrl-Q quit`
will grow as kite does), and `open_file` remembers the name it was
given and says `New file` when there's no such file yet.

## Contract

Nothing new. The tests read the bars off the screen.

## Assignment

- [ ] The text gets the window's rows less two, and the welcome is a third of the way down those
- [ ] The status bar, inverted and exactly as wide as the window: the name (or `[No Name]`), cut to 20 characters, then ` - N lines` on the left; `line/lines` on the right, the line counted from 1. When both don't fit, drop the right side, then cut the left
- [ ] The message bar shows the message, cut to the window's width, for 5 seconds after it was set
- [ ] After `open_file`, the status bar shows the file's name, and a file that isn't there gets the message `New file`
- [ ] kite shows its keys in the message bar when it starts
