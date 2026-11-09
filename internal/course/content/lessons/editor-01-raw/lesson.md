---
title: Raw mode and reading keys
kind: capstone
program: kite
minutes: 35
files: [keys.cpp, raw.cpp]
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
and searches files, highlights C++, and undoes. Each lesson, you write
the core pieces as plain functions the tests can check, and the course
supplies the code around them. Run starts kite in a terminal of its
own, so you can try it as it grows.

A terminal normally hands a program its input a **line** at a time,
after you press Enter, and it echoes what you type. That's **cooked**
mode, done by the kernel's terminal driver, and an editor can't work
like that. It needs every key the moment it's pressed. **Raw mode**
switches the driver's processing off, through the `termios` flags:

```cpp
termios t;
tcgetattr(STDIN_FILENO, &t);       // the current settings
t.c_lflag &= ~(ECHO | ICANON);     // no echo, no line buffering
tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
```

Some keys aren't one byte. The arrows, Home, End and friends arrive as
**escape sequences**: ESC (`\x1b`), then `[`, then a letter or a number
and `~`. Up is `\x1b[A`, Delete is `\x1b[3~`. Run this lesson and press
keys to see them.

## Assignment

- [ ] `raw_attrs` turns off `ECHO`, `ICANON`, `ISIG` and `IEXTEN` in `c_lflag`; `IXON`, `ICRNL`, `BRKINT`, `INPCK` and `ISTRIP` in `c_iflag`; `OPOST` in `c_oflag`; sets `CS8` in `c_cflag`; and sets `VMIN` to 0 and `VTIME` to 1. Nothing else changes
- [ ] `decode_key` returns an ordinary byte as itself (0 to 255), using 1 byte
- [ ] `ESC [ A`/`B`/`C`/`D` are the arrows, `ESC [ H`/`F` and `ESC O H`/`F` are Home and End, and `ESC [ n ~` is Home (1, 7), Delete (3), End (4, 8), Page Up (5) or Page Down (6)
- [ ] Any other whole `ESC [` sequence (digits and `;`, then a byte from `@` to `~`) is `kUnknown`, and uses all its bytes. Other `ESC O x` is `kUnknown` too, using 3
- [ ] ESC alone, an unfinished sequence, or ESC then anything else is just `kEsc`, using 1 byte
