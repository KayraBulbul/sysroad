---
title: Reaping children with SIGCHLD
minutes: 40
files: [reaper.cpp]
program: reaper
run:
  args: []
deeper:
  - title: "CS:APP 8.5.5-8.5.6: handlers, and the addjob/deletejob race"
    url: https://csapp.cs.cmu.edu/
  - title: "wait(2): WNOHANG, and SIGCHLD"
    url: https://man7.org/linux/man-pages/man2/wait.2.html
---
A shell can't sit in `waitpid` while background jobs run: it has a
prompt to show. Last chapter's `reap_finished` polls, so a job is
noticed only when you next look. The kernel offers better: whenever a
child ends, the parent gets **`SIGCHLD`**. Its default action is to
ignore it, so you've been getting them all along.

A handler that reaps on `SIGCHLD` means no zombie ever lingers. It has
a trap, though. Signals don't queue: if three children end while the
first `SIGCHLD` is pending, the handler runs **once**. So it must
reap in a loop, with `WNOHANG`, until there's nothing left:

```cpp
void on_sigchld(int) {
    int saved = errno;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        record pid and status
    }
    errno = saved;   // waitpid ends with ECHILD: don't leak that
}
```

"Record" is the second trap: the handler can't touch a `std::set` or
a `std::vector`, which might be halfway through a change in the main
program. It writes to plain static arrays with a `sig_atomic_t`
count, and the main program moves the entries into real data
structures, with **`SIGCHLD` blocked** while it does, so the handler
can't add one in the middle.

The third trap is a race that CS:APP made famous. You fork, and before
`fork` even returns in the parent, the child might run, end, and be
reaped by the handler. If the parent only then adds the PID to its
"running" set, it adds a child that's already gone, and waits for it
forever. The fix is to block `SIGCHLD` **before** forking and unblock
it after the PID is recorded. The child inherits the blocked mask, so
it must put the old one back before it `exec`s.

To wait for everything to finish, it's `sigsuspend` again: with
`SIGCHLD` blocked, check `running_`, and sleep with it unblocked.

## Assignment

- [ ] The handler reaps every finished child with `WNOHANG` in a loop, storing them in the static arrays, and keeps `errno`
- [ ] `start` blocks `SIGCHLD` around the fork and recording the PID; the child restores the old mask before `exec`
- [ ] `collect` moves the handler's entries into `done_` and out of `running_`, and `finished` and `running` call it with `SIGCHLD` blocked
- [ ] `wait_all` sleeps in `sigsuspend` until nothing is running, and leaves the mask as it found it
