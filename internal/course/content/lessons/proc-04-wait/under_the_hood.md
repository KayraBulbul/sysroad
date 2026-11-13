**Why zombies exist.** A parent might ask how its child ended at any
time, so the kernel can't throw that answer away when the child
exits. It keeps the process's entry, a few hundred bytes, until it's
reaped. A program that starts children and never waits for them
fills the process table with zombies; when it runs out of PIDs,
`fork` fails everywhere. You can't kill a zombie: it's already dead.
Kill its parent, and PID 1 adopts the zombies and reaps them.

**Not caring.** If a parent sets `SIGCHLD` to `SIG_IGN`, or sets the
`SA_NOCLDWAIT` flag, its children are reaped automatically and never
become zombies, at the cost of never learning how they ended. Next
chapter, you'll reap them from a `SIGCHLD` handler instead.

**The bits in wstatus.** On Linux and the Mac, the low 7 bits are the
signal that killed the process (0 if it exited), bit 7 is the
core-dump flag, and the next 8 bits are the exit code. So `exit(3)`
gives 0x0300, and a segfault with a core dump gives 0x008b. A
*stopped* process (Ctrl-Z, later this chapter) gives 0x7f in the low
bits, and `waitpid` only reports it if you pass `WUNTRACED`.

**Exit codes are one byte.** `exit(256)` exits 0, which is why
`return errors.size()` from `main` is a bug. Codes above 125 are taken
by convention: 126 can't run, 127 not found, 128+n killed by signal
n. Keep your own between 1 and 125. `sysexits.h` suggests some (64 for
bad usage, 75 for "try again later"), and plenty of programs ignore
it.

**Core dumps on your machine.** On Linux, `/proc/sys/kernel/core_pattern`
decides where cores go; with systemd it's piped to `systemd-coredump`,
and `coredumpctl list` shows recent crashes with their stack traces.
The Mac writes `.ips` crash reports to `~/Library/Logs/DiagnosticReports`
instead, and real cores to `/cores` only if you raise `ulimit -c`.

**waitid and pidfds.** POSIX's `waitid` reports more (who sent the
signal, the full 32-bit exit status) and has a `WNOWAIT` option that
looks without reaping. On Linux, a pidfd from `pidfd_open` can be put
in `poll` with your other fds, and becomes readable when the process
ends: a way to wait for a child and a socket in the same loop.
