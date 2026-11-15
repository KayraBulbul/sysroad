---
title: Line editing
kind: capstone
project: conch
program: conch
minutes: 55
run:
  interactive: true
deeper:
  - title: "linenoise: a line editor in 1,100 lines of C (antirez)"
    url: https://github.com/antirez/linenoise
  - title: "GNU Readline's default key bindings"
    url: https://tiswww.case.edu/php/chet/readline/rluserman.html#Commands-For-Moving
---
The last lesson. The terminal's own line editing (canonical mode) gives
you Backspace and not much else: no arrow keys, no history, no Ctrl-A.
Every real shell switches the terminal to **raw mode** while you type,
as kite did, and edits the line itself. That's what GNU Readline does
for bash, and linenoise for redis-cli.

**Keys.** You wrote a key decoder for kite: bring it over. Copy your
`decode_key` and your key values into conch's folder by hand (conch is
a project of its own, so nothing comes across by itself), and keep the
idea of your key reader: when what's read so far decodes as Escape,
give the rest of a sequence a moment to arrive, and keep doing so while
it still decodes as Escape and could be the start of one. Over a slow
link `ESC`, `[` and `D` can arrive separately, and `ESC [` alone is
still Escape. A lone ESC, or ESC followed by ordinary text, is Escape
after at most one wait.

**Reading a line**, when conch is talking to a terminal, now happens in
raw mode, switched on for the line and off again before the command
runs: commands expect the terminal as it normally is. Reading waits for
each key (`VMIN` 1, `VTIME` 0 is fine here). Each key goes to the line
being edited, which is plain state, a string and a cursor, with no
terminal in it:

```text
printable       insert at the cursor          Left/Right   move (not past the ends)
Backspace       delete before the cursor      Home, Ctrl-A start of line
Delete          delete at the cursor          End, Ctrl-E  end of line
Ctrl-U          delete to the start           Ctrl-K       delete to the end
Ctrl-W          delete the word before the cursor (spaces first, then non-spaces)
Up/Down         older/newer history lines; Down past the newest gives back
                what you were typing; the cursor goes to the end
Enter           run the line                  Ctrl-C  throw it away, print ^C, start again
Ctrl-D          end of input on an empty line, else Delete
```

Keys can arrive several at once: a paste, or a line typed ahead while
the last command ran. Keep the bytes you've read but not used for the
next line, rather than dropping them when a line ends. Other keys (F5,
Page Up, other control keys) do nothing. Up stops at the
oldest line, and Down does nothing when you aren't in the history.

After every key, redraw the whole line: `\r`, the prompt and the text,
`\x1b[K` to clear what's left of an older, longer line, and if the
cursor isn't at the end, `\x1b[nD` to move it back `n` columns. Enter
writes `\r\n` (raw mode doesn't turn `\n` into `\r\n` for you) and
returns the line. Ctrl-D on an empty line is the end of the input, as
it was in canonical mode: conch writes `\r\n` and ends.

## Contract

Nothing new. The tests type at conch on a terminal, editing keys and
all, and look at what the edited commands print.

## Assignment

- [ ] On a terminal, conch reads each line in raw mode, with your key decoder brought over from kite, and gives the terminal back as it was before running the line
- [ ] Every key in the table edits the line as described, and other keys do nothing
- [ ] Up and Down walk the history, newest first, and Down past the newest gives back what you were typing
- [ ] The line is redrawn after each key, with the cursor in place; Ctrl-C starts a new line, and Ctrl-D on an empty line ends conch
- [ ] Keys typed ahead, or pasted, aren't lost: they go to the next lines
