---
title: Blocking signals and sigsuspend
minutes: 35
files: [mask.cpp]
program: waitsig
run:
  args: []
deeper:
  - title: "CS:APP 8.5.6-8.5.7: synchronizing flows, and waiting for signals"
    url: https://csapp.cs.cmu.edu/
  - title: "sigprocmask(2) man page"
    url: https://man7.org/linux/man-pages/man2/sigprocmask.2.html
  - title: "sigsuspend(2) man page"
    url: https://man7.org/linux/man-pages/man2/sigsuspend.2.html
---
Here's how to wait for a signal, and it's wrong:

```cpp
while (!got_usr1) {   // the signal arrives here, after the check...
    pause();          // ...so this sleeps forever
}
```

`pause` sleeps until a signal arrives. If the signal lands between
the check and the `pause`, the handler sets the flag, but the loop has
already looked, and `pause` waits for a signal that's already been.
It works almost every time, which is the worst kind of bug.

The tool for this is the **signal mask**: the set of signals a process
is currently *blocking*. A blocked signal isn't lost. It stays
**pending** (one bit per signal) and is delivered the moment it's
unblocked.

```cpp
sigset_t set, old;
sigemptyset(&set);
sigaddset(&set, SIGUSR1);
sigprocmask(SIG_BLOCK, &set, &old);   // block it, and remember the old mask
// ... SIGUSR1 can't interrupt this ...
sigprocmask(SIG_SETMASK, &old, nullptr);   // put the old mask back
sigpending(&set);                          // which signals are waiting?
```

Blocking makes the check safe, since the signal can't arrive in the
middle any more. But then you can't sleep with it blocked, and
unblocking it before `pause` brings the gap back. `sigsuspend` closes
it: it swaps in a mask and sleeps **in one step**, and when a handler
has run, it puts your mask back and returns.

```cpp
block SIGUSR1
while (!got_usr1) {
    sigsuspend(&mask_without_usr1);   // unblock + sleep, atomically
}
unblock SIGUSR1
```

If the signal was already pending, `sigsuspend` delivers it and
returns at once. So you can block a signal *before* doing whatever
will cause it, like forking a child that will send it, and never miss
it. That's what `waitsig` does a thousand times.

Blocking is a resource with a "before" and an "after", so it's RAII
again: `BlockSignals` blocks in its constructor and restores the old
mask in its destructor. Then `await_signal` builds on it.

## Assignment

- [ ] `BlockSignals` adds its signals to the mask, and puts the previous mask back when destroyed, so blocks nest
- [ ] `is_pending` asks `sigpending`
- [ ] `await_signal` blocks the signals, installs a handler that notes which one came, and `sigsuspend`s with a mask that lets only them in, until one is noted
- [ ] Afterwards the mask and every action are back as they were, and a signal that was already pending returns at once
