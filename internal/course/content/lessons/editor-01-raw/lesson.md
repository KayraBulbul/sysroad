---
title: Raw mode
kind: capstone
project: kite
program: kite
minutes: 40
contract:
  header: kite.hpp
  functions:
    - name: raw_attrs
      sig: "termios(termios)"
run:
  interactive: true
deeper:
  - title: "Build Your Own Text Editor (Jeremy Ruten), the kilo tutorial"
    url: https://viewsourcecode.org/snaptoken/kilo/
  - title: "termios(3) (man7.org)"
    url: https://man7.org/linux/man-pages/man3/termios.3.html
---
This chapter builds **kite**, a text editor in about a thousand lines,
in the spirit of antirez's `kilo`. By the end it opens, edits, saves
and searches files, highlights C++, and undoes. It's a **project**:
every lesson works in the same folder, which starts empty, and every
line of kite is yours, `main` included. Each lesson names the few
functions its tests call, declared in a header called `kite.hpp`. The
rest of the design (your structs, your files, your helpers) is up to
you. Run starts kite in a terminal of its own, so you can try it as it
grows.

A terminal normally hands a program its input a **line** at a time,
after you press Enter, and it echoes what you type. That's **cooked**
mode, done by the kernel's terminal driver, and an editor can't work
like that. It needs every key the moment it's pressed. **Raw mode**
switches the driver's processing off, through the `termios` flags in
`<termios.h>`:

```cpp
termios t;
tcgetattr(STDIN_FILENO, &t);       // the current settings
t.c_lflag &= ~(ECHO | ICANON);     // no echo, no line buffering
tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
```

Raw mode has to be undone. The settings belong to the terminal, not to
your program, so if kite ends without restoring them, the shell is left
not echoing what you type. That's a job for RAII, as in lesson 1.3.4:
a class whose constructor saves the settings and switches raw mode on,
and whose destructor puts the saved settings back, however `main` ends.

Not every program's input is a terminal: `kite < notes.txt` or a pipe
has none to switch. `isatty(STDIN_FILENO)` says whether it is. Without
one, kite prints an error to stderr and exits with status 1, before it
writes anything to stdout.

Then the loop. With `VMIN` 0 and `VTIME` 1, `read` waits at most a
tenth of a second: it returns 1 with a byte, or 0 with none, and kite
just reads again. For now kite prints each byte it gets, so you can see
what your keys send, and **Ctrl-Q** ends it. Ctrl clears the top three
bits of a letter, so Ctrl-Q is `'q' & 0x1f`, which is 17. Two things
about printing in raw mode:

- With `OPOST` off, `\n` moves down a line without going back to the
  left edge. End lines with `\r\n`.
- `std::cout` keeps what you print in a buffer. Flush it after each
  line (`<< std::flush`), or nothing shows until the buffer fills.

Every `.cpp` file in the folder is compiled into kite, so name and
split your files as you like. The tests compile them too, with your
`main` renamed, then start it on a terminal of their own (a
pseudo-terminal) and check the terminal's settings while it runs. They
call it as `main(argc, argv)`, so give it both parameters, though kite
won't look at them until it opens files. And end it with `return 0;`:
only the real `main` returns 0 when it runs off its end, and the
renamed one is an ordinary function.

## Contract

```cpp
// kite.hpp
termios raw_attrs(termios t);  // the settings for raw mode, made from t
```

And in a `.cpp` file: `int main(int argc, char** argv)`.

## Assignment

- [ ] Create `kite.hpp` with `raw_attrs`, and write it in a `.cpp` file
- [ ] `raw_attrs` turns off `ECHO`, `ICANON`, `ISIG` and `IEXTEN` in `c_lflag`; `IXON`, `ICRNL`, `BRKINT`, `INPCK` and `ISTRIP` in `c_iflag`; `OPOST` in `c_oflag`; sets `CS8` in `c_cflag`; and sets `VMIN` to 0 and `VTIME` to 1. Nothing else changes
- [ ] kite switches the terminal to raw mode as it starts, and puts the old settings back however it ends
- [ ] Without a terminal, it prints an error to stderr, writes nothing to stdout, and exits with status 1
- [ ] It prints each byte it reads on a line of its own, and Ctrl-Q ends it with status 0
