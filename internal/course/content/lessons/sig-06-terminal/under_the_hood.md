**What a terminal is now.** A pseudo-terminal is a pair of devices:
the *master*, held by your terminal emulator (or SSH, or tmux), and
the *slave*, `/dev/pts/N`, which your shell has as fds 0, 1 and 2.
Between them sits the kernel's **line discipline**: it echoes what
you type, buffers a line until Enter (canonical mode), handles
Backspace, and turns Ctrl-C and Ctrl-Z into signals for the
foreground group. `stty -a` shows its settings. In raw mode (your text
editor in phase 3) it does none of that.

**Why both sides call tcsetpgrp.** The same race as `setpgid`: the
child might `exec` and start reading the terminal before the parent
hands it over, and get `SIGTTIN` for its trouble. Calling it in the
child before `exec` closes that gap; the parent's call covers the case
where the parent's next step needs the job in the foreground first.

**Starting in the background.** The loop at the top of your
`init_job_control` is straight from bash's `jobs.c` and the glibc
manual's sample shell. Without it, `bash &` would take the terminal
from the shell you typed it into, and both would fight over your
keystrokes.

**TOSTOP.** By default background jobs may *write* to the terminal,
which is why a background job's output scrambles your prompt.
`stty tostop` makes background writes raise `SIGTTOU` too, stopping
the job until you `fg` it.

**SIGCONT is special.** It continues a stopped process even if it's
blocked or ignored, and it can be sent to any process in your session
regardless of user. A stopped process still receives `SIGKILL`, but
nothing else is acted on until it's continued: that's why shells send
`SIGCONT` after `SIGTERM` when they kill a stopped job with `kill %1`.

**How your terminal emulator does it.** It calls `posix_openpt` (or
`openpty`), forks, and in the child calls `setsid`, opens the slave so
it becomes the controlling terminal, puts it on 0, 1 and 2, and
`exec`s your shell. The tests' `Pty` class does exactly that, then
reads the master as your terminal would.
