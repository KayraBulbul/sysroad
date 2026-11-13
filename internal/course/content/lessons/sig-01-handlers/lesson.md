---
title: Signals and handlers
minutes: 30
files: [signals.cpp]
program: ticker
run:
  args: ["-t", "1"]
deeper:
  - title: "CS:APP 8.5: signals"
    url: https://csapp.cs.cmu.edu/
  - title: "signal(7): every signal and its default action"
    url: https://man7.org/linux/man-pages/man7/signal.7.html
  - title: "sigaction(2) man page"
    url: https://man7.org/linux/man-pages/man2/sigaction.2.html
---
A **signal** is a tiny message from the kernel to a process: just a
number, with no data. Some come from the kernel itself, when the
process does something wrong (`SIGSEGV` for a bad address, `SIGFPE`
for dividing by zero) or something happens (`SIGCHLD` when a child
ends, `SIGPIPE` from last chapter). Others come from other processes,
with `kill`:

```cpp
kill(pid, SIGTERM);   // please stop
raise(SIGUSR1);       // send one to yourself
```

and Ctrl-C at the terminal is the terminal driver sending `SIGINT`.

Every signal has a **default action**: most end the process
(`SIGTERM`, `SIGINT`, `SIGPIPE`, `SIGUSR1`), some also dump core
(`SIGSEGV`, `SIGQUIT`, `SIGABRT`), some are ignored (`SIGCHLD`), and
some stop or continue it (Ctrl-Z's `SIGTSTP`, `SIGCONT`). A process can
change the action to ignore the signal (`SIG_IGN`) or to call a
function, a **handler**, except for `SIGKILL` and `SIGSTOP`, which
always do what they say.

The old way to set one, `signal(sig, handler)`, behaves differently
on different systems. Use `sigaction`, which spells everything out:

```cpp
struct sigaction sa{};
sa.sa_handler = on_term;      // or SIG_IGN, or SIG_DFL
sa.sa_flags = SA_RESTART;     // see below
sigemptyset(&sa.sa_mask);     // signals to block while it runs (next lessons)
sigaction(SIGTERM, &sa, &old);
```

A handler can run **at any moment**, between any two instructions of
your program, in the middle of anything. Almost nothing is safe to do
there (that's the next lesson). The one thing that always is: set a
flag of type `volatile sig_atomic_t`, and let the main program look at
it when it's ready.

```cpp
volatile sig_atomic_t stop_signal = 0;
void stop_handler(int sig) { stop_signal = sig; }
```

`volatile` stops the compiler from keeping the flag in a register
through your loop, and `sig_atomic_t` is an integer that can be read
and written in one step.

When a signal arrives while the process is blocked in a system call,
like `read` on an empty pipe or `usleep`, the handler runs and the
call is cut short: it fails with `EINTR`. That's how a `SIGINT` can
wake a sleeping program to look at its flag. With `SA_RESTART`, the
kernel restarts most calls instead, as if nothing happened (`usleep`
and `poll` never restart). Choose per signal: restarting is easier for
the rest of the program, but then the signal can't interrupt it.

`ticker` ticks until a signal asks it to stop: Ctrl-C, `kill`, or an
`alarm`, which sends `SIGALRM` after some seconds.

## Assignment

- [ ] `install_handler` fills in a `sigaction` with the handler, the flags and an empty mask, installs it, and returns the old one, throwing `std::system_error` if it fails
- [ ] `stop_handler` only records the signal
- [ ] `tick_until_stopped` checks the flag before each tick, so a sleep cut short by a stop signal isn't counted
- [ ] Run `ticker` without `-t`, then stop it with Ctrl-C, and from another terminal with `kill`
