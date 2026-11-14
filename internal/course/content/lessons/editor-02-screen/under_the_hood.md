**Where the codes came from.** The DEC VT100 terminal (1978) used these
sequences, and ANSI standardised them as X3.64. Physical terminals
are gone, but every terminal emulator (Alacritty, Ghostty, Terminal.app,
the Linux console) still speaks VT100, plus decades of additions.

**The alternate screen.** `\x1b[?1049h` switches to a second screen
buffer, and `\x1b[?1049l` switches back. That's how vim, less and
kite leave your shell's scrollback untouched when they exit. Programs
that crash on the alternate screen leave you there; `tput rmcup` gets
you back.

**Why redraw everything?** It's simple and, at terminal sizes, fast:
a 50-row screen is a few kilobytes. Real editors are smarter. ncurses
keeps a copy of what's on screen and sends only the differences, which
mattered on 9600-baud lines. Modern terminals are fast enough that
kilo-style editors get away without it.

**Synchronised output.** Even one `write` can be split by the terminal
mid-frame. Newer terminals support `\x1b[?2026h` ... `\x1b[?2026l`
around a frame, which tells the terminal to show it all at once.

**Window size.** `ioctl(fd, TIOCGWINSZ, &ws)` asks the terminal driver
for the size. kite asks before every frame, so resizing the window just
works, but only once you press a key. Programs that redraw straight
away listen for `SIGWINCH`, the signal sent when the size changes.
Where there's no `ioctl` to ask, kilo moves the cursor to the far
bottom right (`\x1b[999C\x1b[999B`) and asks the terminal where it is
(`\x1b[6n`), which answers `\x1b[24;80R`.

**How the tests see your screen.** They feed what `draw_screen`
returns to a small terminal emulator of their own, a grid of
characters with a cursor, and compare the grid. `\x1b[H` and
`\x1b[1;1H` land in the same place, so either passes.
