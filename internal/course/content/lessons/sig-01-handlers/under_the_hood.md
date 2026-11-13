**How delivery works.** Sending a signal just sets a bit in the
target's *pending* set. The kernel checks that set whenever the
process is about to return from kernel mode to user mode: after a
system call, an interrupt, or a page fault. If a pending signal has a
handler, the kernel builds a frame on the process's stack holding
everything needed to resume, and returns into the handler instead of
where the process was. The handler returns into a tiny trampoline
that calls `sigreturn`, and the kernel restores the saved state.
`strace` shows `rt_sigreturn` after every handler.

**Signals don't queue.** Pending is one bit per signal. Ten
`SIGCHLD`s sent while one is pending are still one `SIGCHLD`. That
matters a lot in lesson 4. POSIX real-time signals (`SIGRTMIN` to
`SIGRTMAX`, Linux only) do queue, and carry a value with `sigqueue`.

**Where Ctrl-C comes from.** The terminal driver turns the byte 0x03
into `SIGINT` for every process in the terminal's foreground process
group (lesson 6). Ctrl-\ (0x1c) sends `SIGQUIT`, which also dumps
core: a quick way to get a backtrace out of a hung program. `stty -a`
shows these settings; `intr = ^C` is the one.

**What Go and Python do.** Python installs a C handler for `SIGINT`
that only sets a flag, and the interpreter checks it between bytecodes
and raises `KeyboardInterrupt`: exactly the flag pattern. Go's runtime
catches every signal on a dedicated stack and turns them into
messages on the channels you gave `signal.Notify`, so in Go you never
write a real handler at all.

**Graceful shutdown.** `systemctl stop`, `docker stop` and Kubernetes
all send `SIGTERM`, wait (10 s for Docker, 30 s by default for
Kubernetes), then send `SIGKILL`. A server that catches `SIGTERM`,
stops accepting work and finishes what it has is a server that never
loses a request on deploy. `ticker` is the smallest version of that.
