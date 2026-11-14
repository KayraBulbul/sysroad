---
title: Drawing the screen with escape codes
kind: capstone
project: kite
program: kite
minutes: 50
contract:
  types: [Editor]
  functions:
    - name: set_window_size
      sig: "void(Editor&, int, int)"
    - name: open_file
      sig: "void(Editor&, const std::string&)"
    - name: draw_screen
      sig: "std::string(Editor&)"
retires:
  - lesson: editor-01b-keys
    cases: ["kite prints the name of each key*", "an arrow that arrives in two pieces*", "Escape on its own*"]
run:
  args: [demo/poem.txt]
  interactive: true
deeper:
  - title: "VT100 escape sequences (vt100.net)"
    url: https://vt100.net/docs/vt100-ug/chapter3.html
  - title: "ANSI escape code (Wikipedia)"
    url: https://en.wikipedia.org/wiki/ANSI_escape_code
---
This lesson kite starts to look like an editor: `kite poem.txt` shows
the file, and Ctrl-Q quits. The key names go, and last lesson's tests
of them retire with them.

A terminal draws whatever bytes it's sent, and some byte sequences are
commands instead of text. They start with ESC `[`, like the keys did,
and they move the cursor, clear lines and change colours:

```text
\x1b[H          move the cursor to the top left (row 1, column 1)
\x1b[12;5H      move it to row 12, column 5 (counting from 1)
\x1b[K          clear from the cursor to the end of the line
\x1b[?25l       hide the cursor; \x1b[?25h shows it again
```

**The editor's state.** kite has to remember the text, the cursor and
the window's size, in a struct called `Editor`. Its design is yours. A
good start:

```cpp
struct Row {                 // one line of the text
    std::string chars;
};

struct Editor {
    std::vector<Row> rows;
    int cx = 0, cy = 0;      // the cursor: column and row in the text
    int screen_rows = 24;
    int screen_cols = 80;
};
```

A `Row` holding one string looks like overkill, but lines will soon
carry more than their text. The default values matter: the tests make
an editor with `Editor e;`. `open_file` reads a file's lines into it
with `std::getline`, which drops each `\n`. A file that isn't there
opens empty: you might be about to create it.

**Drawing.** kite redraws the whole screen after every key.
`draw_screen` builds the frame as one string, and `main` writes it in a
single `write`. Lots of small writes would let the terminal show
half-drawn frames, which flicker. Each row of the window is a line of
the text, cut at the window's width, or a `~` past the end of the file,
the way vi shows it. Each row ends with `\x1b[K` to wipe what the last
frame left there, then `\r\n`, except the bottom row: a newline there
would scroll the whole screen up a line. Hide the cursor while drawing,
so it doesn't flash across the screen, then move it to `(cx, cy)`,
which is the top left until next lesson, and show it.

A file with no lines shows a welcome a third of the way down, at row
`rows / 3`, centred:

```text
~
~      kite -- a tiny text editor
~
```

`draw_screen` takes a plain `Editor&`, not a `const` one: in a few
lessons it will move the view before it draws.

**The main loop.** `main` becomes the editor's loop:

```text
open the file named on the command line, if there is one
raw mode, the alternate screen, a key reader
loop:  ask the terminal its size  ->  set_window_size
       draw_screen  ->  write it
       read a key; Ctrl-Q ends the loop
```

Three pieces of terminal plumbing go with it:

- **The size.** `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)`, from
  `<sys/ioctl.h>`, fills a `winsize` with `ws_row` and `ws_col`. Ask
  before every frame, and resizing the window just works. If it fails,
  or says 0 columns, assume 24 by 80.
- **Writing.** `write` can write fewer bytes than you asked for, or be
  interrupted before it writes any (`EINTR`). A loop writes the rest.
- **The alternate screen.** `\x1b[?1049h` switches the terminal to a
  second screen and `\x1b[?1049l` switches back, which is how vim and
  less leave your shell's screen as it was. It's another RAII class,
  made after the raw-mode one, so that without a terminal kite still
  writes nothing.

## Contract

```cpp
struct Editor { /* yours */ };

// The terminal window is rows x cols.
void set_window_size(Editor& e, int rows, int cols);

// Loads the file at path, a line per row. A file that isn't there opens empty.
void open_file(Editor& e, const std::string& path);

// Everything to write to the terminal to show the editor.
std::string draw_screen(Editor& e);
```

The tests make an `Editor e;`, set its size and open a file, then feed
what `draw_screen` returns to a small terminal of their own and look at
the screen it shows. Any escape codes that draw the right screen pass.

## Assignment

- [ ] `open_file` loads the file's lines, and a missing file opens empty
- [ ] `draw_screen` draws every row of the window: the text's line, cut at the window's width, or `~` past the end of the text, with nothing left over from the last frame
- [ ] With no text at all, the row at `rows / 3` is `kite -- a tiny text editor`, cut to the width and centred: the spare columns halved (rounding down) to the left, the first of them a `~`
- [ ] The frame ends with the cursor showing, at its place in the text (the top left, for now), and doesn't scroll the screen
- [ ] `kite FILE` shows the file on the alternate screen, fitted to the terminal, until Ctrl-Q puts the shell's screen back
