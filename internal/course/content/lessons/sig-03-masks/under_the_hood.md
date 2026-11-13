**The kernel's side.** Each thread has a blocked set and a pending
set, and delivery is just "pending and not blocked". `sigprocmask`
and `sigsuspend` are a few bit operations. `sigsuspend` always
returns -1 with `EINTR`, since returning at all means a handler ran.

**Masks while a handler runs.** While a handler runs, its own signal
is blocked automatically (unless you pass `SA_NODEFER`), and
`sa_mask` blocks more. That's why the same handler can't interrupt
itself. Use `sa_mask` when two handlers touch the same data: let each
block the other.

**Masks and threads.** Each thread has its own mask, and a signal sent
to the process goes to any one thread that isn't blocking it. Use
`pthread_sigmask` in threaded code (`sigprocmask`'s behaviour there
is unspecified). The robust pattern for threaded programs: block
everything in `main` before starting threads, so they all inherit the
block, and dedicate one thread to `sigwait`, which waits for a
blocked signal and *returns* it, with no handler at all.

**Masks survive fork and exec.** A child inherits the parent's mask
and pending signals are cleared; `exec` keeps the mask. So a child
must unblock what it needs before `exec`, or the program it runs
starts life with, say, `SIGCHLD` blocked, which some programs never
check. `posix_spawn` has `POSIX_SPAWN_SETSIGMASK` for this.

**pselect and ppoll.** The same race exists for "check the flag, then
`poll`". `ppoll` and `pselect` take a signal mask to use while
waiting, atomically, exactly like `sigsuspend`. The self-pipe from
last lesson is the other answer, and works everywhere.
