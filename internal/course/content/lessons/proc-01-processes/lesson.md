---
title: What a process is
minutes: 25
files: [self.cpp]
program: self
run:
  args: []
deeper:
  - title: "CS:APP 8.1-8.2: exceptions and processes"
    url: https://csapp.cs.cmu.edu/
  - title: "proc(5): the /proc file system"
    url: https://man7.org/linux/man-pages/man5/proc.5.html
  - title: "environ(7) man page"
    url: https://man7.org/linux/man-pages/man7/environ.7.html
---
In Go you call `exec.Command`, in Python `subprocess.run`, and a
program runs. This phase is what happens underneath, and by the end of
it you'll have written the thing that sits on top of all of it: a
shell.

A **process** is a running program, and the kernel keeps a little
bundle of state for each one:

- a **PID**, its number, and its parent's PID (the **PPID**): every
  process was started by another one
- its own **address space**: code, globals, heap, stack. No other
  process can see it
- a table of **open file descriptors**: 0, 1 and 2 are stdin, stdout
  and stderr, and everything it opens takes the next free number
- a **current directory**, which relative paths start from
- an **environment**: a list of `NAME=value` strings, like `PATH` and
  `HOME`
- a user and group it runs as, limits, signal settings and more

All of it is yours to ask about:

```cpp
#include <unistd.h>

pid_t me = getpid();        // pid_t is a plain int
pid_t parent = getppid();   // the shell, if you ran it from one
char buf[4096];
getcwd(buf, sizeof buf);    // nullptr if it doesn't fit
extern char** environ;      // {"PATH=/usr/bin:/bin", "HOME=...", ..., nullptr}
```

`environ` is a global the C library sets up before `main` runs: an
array of C strings, ending with a null pointer. `getenv("HOME")` just
searches it.

The open fds you can list as a folder. `/dev/fd` holds one entry per
open fd, named by its number, on both Linux and the Mac. On Linux
it's a link to `/proc/self/fd`, and `/proc/<pid>/` holds the rest of a
process's state as files too: try `ls -l /proc/self/fd` and
`cat /proc/self/status`.

Reading a folder in C is `opendir`, then `readdir` until it returns
null, then `closedir`. Careful, though: `opendir` opens an fd of its
own, so `/dev/fd` lists that one as well, along with `.` and `..`.
`dirfd(dir)` tells you which number it got.

## Assignment

- [ ] `open_fds` lists the open fds from `/dev/fd`, sorted, without `.`, `..` or the folder's own fd
- [ ] `environment` splits each `environ` entry at its first `=` into a map
- [ ] `current_process` fills in the PID, the parent's PID, the current directory and the open fds
- [ ] Run it, then run `ps -o pid,ppid,comm` in the same terminal and find your shell, its parent
