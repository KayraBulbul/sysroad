---
title: Async-signal safety
minutes: 35
files: [safe.cpp]
program: events
run:
  stdin: "hello\nraise USR1\nraise USR2\nraise USR2\nbye\nraise TERM\nnever seen\n"
deeper:
  - title: "signal-safety(7): the list of safe functions"
    url: https://man7.org/linux/man-pages/man7/signal-safety.7.html
  - title: "CS:APP 8.5.5: writing signal handlers"
    url: https://csapp.cs.cmu.edu/
  - title: "The self-pipe trick (D. J. Bernstein)"
    url: https://cr.yp.to/docs/selfpipe.html
---
Why only set a flag in a handler? Because it can interrupt **anything**,
including the code it calls. Say `main` is inside `malloc`, halfway
through updating its free lists, when a signal arrives, and the
handler calls `printf`, which calls `malloc`. The second `malloc` finds
the lists half-updated and corrupts them, or waits for a lock that
the interrupted `malloc` holds and will never release. The program
hangs or crashes, maybe once a month.

A function is **async-signal-safe** if it's fine to call from a
handler. POSIX lists them, and they're mostly raw system calls:
`write`, `read`, `_exit`, `kill`, `waitpid`, `sigaction`. Not on the
list: `printf`, `malloc` and `new`, `std::string`, `std::cout`,
`exit`, and anything that might use them. That includes most of C++.

So a handler that has to say something formats it by hand into a
buffer on the stack, and `write`s it. And because a handler runs
between two arbitrary instructions, it must leave `errno` as it found
it: if `main` just got -1 from a call and is about to look at `errno`,
a handler's own failed `write` would quietly change the answer. Save
it on the way in, put it back on the way out.

```cpp
void on_usr2(int) {
    int saved = errno;
    char buf[64];
    size_t n = format_line(buf, sizeof buf, "got SIGUSR2 #", ++count);
    write(1, buf, n);
    errno = saved;
}
```

A flag has a problem too. A program waiting in `poll` for input won't
look at its flag until some input arrives. The **self-pipe trick**
fixes that: make a pipe, and have the handler write one byte, the
signal's number, into it. Now the signal is just another fd for `poll`
to watch, and the main loop handles it in its own good time, where
every function is allowed:

```text
handler:    write(pipe_write_end, &sig, 1)
main loop:  poll({stdin, pipe_read_end}) → read the byte → deal with it
```

Make both ends **non-blocking**. If the pipe ever filled up, a
blocking `write` in the handler would sleep forever, waiting for the
main loop that it interrupted. A dropped byte is better than that.

`events` echoes its input, and the line `raise USR1` sends it that
signal. `SIGUSR1`, `SIGINT` and `SIGTERM` go through a `SignalPipe`;
`SIGUSR2` is logged straight from its handler with `format_line`.

## Assignment

- [ ] `format_line` writes the text, the number and a newline, cut off at `size`, with no allocation and no library calls; mind negative numbers and `LONG_MIN`
- [ ] `SignalPipe` makes a pipe with both ends non-blocking and close-on-exec, and installs a handler with `SA_RESTART` for each signal
- [ ] The handler writes the signal's number as one byte, and saves and restores `errno`
- [ ] `next` returns the next signal or 0 without blocking, and the destructor puts back the old actions and closes both ends
