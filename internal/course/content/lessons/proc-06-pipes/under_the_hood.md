**A pipe is a ring buffer.** On Linux it's 16 pages (64 KB) of kernel
memory, used in a circle; `fcntl(fd, F_SETPIPE_SZ, n)` can grow it to
1 MB. macOS starts pipes at 16 KB and grows them to 64 KB when there's
a lot to buffer. Writes of up to `PIPE_BUF` bytes (4096 on Linux, 512
on the Mac) are **atomic**: several writers' small writes never
interleave mid-message. Bigger writes can be split up.

**Reading with nothing to read.** An empty pipe blocks `read`, unless
the fd is non-blocking, in which case `read` fails with `EAGAIN`.
`read` returns 0 only when the pipe is empty *and* no write end is
open anywhere. That's why one forgotten fd hangs a whole pipeline:
the forgetful process might, as far as the kernel knows, still write
something.

**poll, select and epoll.** `select` (1983) takes bitmaps of fds and
can't handle fds above 1023. `poll` takes an array, and scans all of
it on every call. Linux's `epoll` and the BSDs' and macOS's `kqueue`
register fds once and return only the ready ones, so they stay fast
with 100,000 connections. That's phase 9's event loop. For two fds,
`poll` is perfect, and it works everywhere.

**Named pipes.** `mkfifo /tmp/p` makes a pipe with a name in the file
system. One process opens it to write, another to read, and they
needn't be related. Shells use them (or `/dev/fd/N`) for
`diff <(sort a) <(sort b)`, process substitution, where a command
needs a *file name* rather than stdin.

**What Python and Go do.** Python's `communicate` uses `selectors`
(poll or select) much like yours, and its documentation warns that
`.stdin.write` followed by `.stdout.read` can deadlock. Go's `cmd.Output`
starts a goroutine to copy each pipe, so the blocking happens on
separate threads instead. Python sets `SIGPIPE` to ignored at
start-up and sets it back in the child (`restore_signals=True`). Go
never ignores it: it catches it, and only lets it kill the program
for a write to fd 1 or 2. A caught signal goes back to its default at
`exec` anyway, since the handler is gone.
