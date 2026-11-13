---
title: fork
minutes: 30
files: [spawn.cpp]
program: forks
run:
  args: []
deeper:
  - title: "CS:APP 8.4: process control"
    url: https://csapp.cs.cmu.edu/
  - title: "fork(2) man page"
    url: https://man7.org/linux/man-pages/man2/fork.2.html
  - title: "A fork() in the road (Baumann et al., HotOS 2019)"
    url: https://www.microsoft.com/en-us/research/publication/a-fork-in-the-road/
---
There's one way to make a new process on Unix, and it's strange:

```cpp
pid_t pid = fork();
if (pid == -1) { /* failed: too many processes, or out of memory */ }
if (pid == 0)  { /* the child */ }
else           { /* the parent; pid is the child's PID */ }
```

`fork` is called once and **returns twice**. It makes a copy of the
calling process: the same code, the same memory, the same open fds,
paused at the same point, inside `fork`. Then both carry on. The only
difference is what `fork` returns: 0 in the child, and the child's PID
in the parent.

"The same memory" means a *copy*. After the fork, a variable the child
changes is changed only in the child. The kernel doesn't copy it all
up front, though: both processes share every page, marked read-only,
until one of them writes, and only then is that page copied (**copy on
write**). So a fork is fast, even of a big process.

A child ends with `_exit(code)`, with the underscore, and the parent
collects the code with `waitpid`. You're given `wait_exit_code` for
that, and lesson 4 takes it apart. Why not `exit`, or a `return`? The
child is a copy of the *whole* program, so returning from the code
that forked it would run the rest of the parent's program twice, and
`exit` runs the parent's cleanup a second time: static destructors,
`atexit` handlers, and the flushing of stdio buffers.

Those buffers are a classic trap. `printf` and `std::cout` collect
output in your process's memory and write it out later. Anything
waiting there when you fork is copied too, and both processes write
it out:

```cpp
printf("hello ");   // sitting in the buffer, not written yet
fork();
printf("world\n");  // prints "hello world" twice
```

So flush every stream (`std::fflush(nullptr)`) **before** forking,
and in the child, flush again before `_exit`, or whatever the child
printed is lost.

You'll write `spawn`, which runs a function in a child, and
`run_all`, which runs several at once: in Python, that's roughly
`multiprocessing.Process`.

## Assignment

- [ ] `spawn` flushes stdio, forks, and throws `std::system_error` if the fork fails
- [ ] The child runs `body`, flushes stdio and `_exit`s with its result, or with 1 if it throws
- [ ] `run_all` spawns all `n` children before it waits for any, and returns their codes in order
- [ ] Run it, then pipe it through `cat`. Comment out the first flush and try both again
