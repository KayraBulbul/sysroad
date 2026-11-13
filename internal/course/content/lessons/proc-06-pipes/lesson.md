---
title: Pipes
minutes: 45
files: [capture.cpp]
program: pipes
run:
  args: []
deeper:
  - title: "pipe(7): pipe capacity, atomicity and SIGPIPE"
    url: https://man7.org/linux/man-pages/man7/pipe.7.html
  - title: "poll(2) man page"
    url: https://man7.org/linux/man-pages/man2/poll.2.html
  - title: "Python's subprocess.communicate, and its warning about deadlocks"
    url: https://docs.python.org/3/library/subprocess.html#subprocess.Popen.communicate
---
A **pipe** is a one-way channel in the kernel, with an fd at each end:

```cpp
int p[2];
pipe(p);   // p[0] is the read end, p[1] the write end
```

Bytes written to `p[1]` come out of `p[0]`, in order. Because they're
fds, they survive `fork` and `exec`. So to capture what a command
prints, you make a pipe, fork, and in the child `dup2` the write end
onto fd 1 before the `exec`. The command writes to stdout as usual;
the parent reads the other end. That's `$(command)` in a shell and
`subprocess.check_output` in Python.

Three rules make pipes work, and breaking any of them hangs your
program or kills it:

1. **Close every end you don't use**, in both processes. `read`
   returns 0 (end of file) only once *every* write end is closed,
   everywhere. If the parent forgets its copy of `p[1]`, it waits for
   more output forever, since it could still write some itself.
2. **A pipe holds about 64 KB.** Once it's full, the writer blocks
   until someone reads. So read all the output *before* you
   `waitpid`: a child with more to say than 64 KB can't finish until
   you've read it, and you can't read while you're waiting for it to
   finish.
3. **Writing to a pipe with no readers raises `SIGPIPE`**, which kills
   the writer. That's how `yes | head -1` stops: `head` exits, and
   `yes`'s next write kills it. It's what you want for `yes`, and not
   what you want for your program feeding a command that stopped
   reading early. Python ignores `SIGPIPE` (and Go, for most fds), so
   the `write` fails with `EPIPE` instead.

That last one has a sting in the tail: a signal you ignore stays
ignored across `exec`. So a child must set `SIGPIPE` back to
`SIG_DFL` before it `exec`s, or `yes` would loop forever in a
pipeline you started.

Rule 2 bites hardest when you both write a command's stdin *and* read
its stdout. Write all the input first, and once the command has
filled its output pipe, it stops reading; your write blocks, and
neither side ever moves again. The fix is to do both at once, each as
soon as it can make progress, with `poll`:

```cpp
pollfd fds[2] = {{out_fd, POLLIN, 0}, {in_fd, POLLOUT, 0}};
poll(fds, 2, -1);   // sleeps until at least one of them is ready
// fds[i].revents says which: POLLIN (or POLLHUP) to read, POLLOUT to write
```

Set the write end to non-blocking (`O_NONBLOCK`) so a write takes
what fits and returns instead of blocking. When all the input is
written, close the write end: that's how the command sees its end of
file.

## Assignment

- [ ] `capture` sends the command's stdout through a pipe, reads it to the end, then waits
- [ ] `communicate` feeds the input and reads the output at the same time with `poll`, closing the command's stdin once the input is all written
- [ ] It ignores `SIGPIPE` while it writes, treats `EPIPE` as "stop writing", and puts the old `SIGPIPE` action back afterwards
- [ ] Every child sets `SIGPIPE` back to `SIG_DFL`, and no pipe end leaks, in the parent or into a child
