**Why a terminal works this way.** The terminal driver dates from
teletypes: a printer with a keyboard, wired to a computer. Cooked mode
gave every program line editing (Backspace, Ctrl-U) for free. `ICRNL`
exists because the Enter key on a teletype sent a carriage return,
`OPOST` because moving to a new line took two motions: return the
carriage (`\r`), then feed the paper (`\n`). With `OPOST` off, a plain
`\n` moves down without going back to the left edge, which is why kite
writes `\r\n`.

**Ctrl is a bit mask.** `Ctrl-A` is 1 and `Ctrl-Z` is 26: Ctrl clears
the top three bits of the letter. That's also why `Ctrl-M` is Enter
(13, `\r`), `Ctrl-I` is Tab and `Ctrl-[` is ESC. They're the same bytes.

**ISIG, IXON.** With `ISIG` on, the driver turns Ctrl-C into SIGINT and
Ctrl-Z into SIGTSTP before your program sees them. With `IXON` on,
Ctrl-S pauses the terminal's output until Ctrl-Q, a flow-control
leftover that still freezes people's terminals today. kite wants both
keys, for Save and Quit.

**`VMIN` and `VTIME`.** With `VMIN = 0, VTIME = 1`, `read` returns after
at most a tenth of a second, with or without input. With `VMIN = 1,
VTIME = 0` it would wait for a byte however long that took, which is
simpler but leaves kite no way to notice that nothing came. The next
lesson uses the timeout to tell the Escape key from the start of an
escape sequence. Vim's `ttimeoutlen` setting is the same idea.

**Restoring the terminal.** A program that crashes in raw mode leaves
the shell without echo. `reset` (typed blind) fixes it. That's why
raw mode is RAII: the destructor restores the saved settings on any
way out of `main`, including an exception. Not on every way out of the
program, though: `std::exit` skips the destructors of local objects,
and a crash or `kill -9` skips everything. Editors that care also put
the terminal back from a signal handler.
