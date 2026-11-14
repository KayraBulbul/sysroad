---
title: History and line editing
base: shell-07-expand
kind: capstone
program: conch
minutes: 60
files: [lineedit.cpp, history.cpp]
run:
  interactive: true
deeper:
  - title: "linenoise: a line editor in 1,100 lines of C (antirez)"
    url: https://github.com/antirez/linenoise
  - title: "GNU Readline's default key bindings"
    url: https://tiswww.case.edu/php/chet/readline/rluserman.html#Commands-For-Moving
---
The last milestone. The terminal's own line editing (canonical mode)
gives you Backspace and not much else: no arrow keys, no history, no
Ctrl-A. Every real shell switches the terminal to **raw mode** while
you type, as kite did in phase 3, and edits the line itself. That's
what GNU Readline does for bash, and linenoise for redis-cli.

The course's `terminal.cpp` has the raw-mode part: `read_line` puts the
terminal in raw mode, reads keys with a decoder like kite's (`keys.hpp`),
hands each one to a `LineEditor`, writes what its `render` returns, and
puts the terminal back before the command runs. The editor itself is
yours, in `lineedit.cpp`. It's plain state, with no terminal in it, so
it's easy to test:

```text
printable       insert at the cursor          Left/Right   move (not past the ends)
Backspace       delete before the cursor      Home, Ctrl-A start of line
Delete          delete at the cursor          End, Ctrl-E  end of line
Ctrl-U          delete to the start           Ctrl-K       delete to the end
Ctrl-W          delete the word before the cursor (spaces first, then non-spaces)
Up/Down         older/newer history entries; Down past the newest gives back
                what you were typing; the cursor goes to the end
Enter           Done      Ctrl-C  Cancelled
Ctrl-D          EndOfInput on an empty line, else Delete
```

`render(prompt)` redraws the whole line: `\r`, the prompt and the
text, `\x1b[K` to clear what's left of an older, longer line, and if
the cursor isn't at the end, `\x1b[nD` to move it back `n` columns.

History lives in `sh.history`, oldest first. `main` calls your
`add_history` with every line read, which skips blank lines and a line
equal to the one before, and keeps the newest 1000. The `history`
builtin prints them numbered from 1 (`%5zu` then two spaces), or only
the last N with `history N`. In interactive mode, `main` loads
`~/.conch_history` at the start and saves it at the end: your
`load_history` and `save_history`. Save to a temporary file and
`rename` it over the old one, so a crash never leaves half a history.
History is private (people paste passwords at prompts by mistake), so
create the file with mode `0600`, owner only, as bash does.

## Assignment

- [ ] `LineEditor::feed` handles every key in the table, and `render` draws the line and places the cursor
- [ ] `add_history` skips blanks and repeats and keeps 1000; `history` and `history N` print them numbered
- [ ] `load_history` and `save_history`, saving through a temporary file with mode `0600` and `rename`
- [ ] Use conch for a while: it's a shell now
