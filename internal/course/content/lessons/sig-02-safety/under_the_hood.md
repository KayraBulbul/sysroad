**Why malloc deadlocks.** glibc's `malloc` takes a lock on its arena.
If a handler interrupts `malloc` and calls `malloc` itself on the same
thread, it waits for a lock its own thread holds, and that thread
can't continue until the handler returns. A classic real-world case:
OpenSSH's "regreSSHion" (CVE-2024-6387) was a `SIGALRM` handler that
called `syslog`, which calls `malloc`. Racing it could corrupt the
heap and give an attacker root.

**Reentrant isn't the same.** A *reentrant* function can be
interrupted and called again on the same thread safely (no static
state). A *thread-safe* one can run on several threads at once, often
by taking a lock. Locks make functions thread-safe and signal-unsafe
at the same time, which is why `printf` is one and not the other.

**signalfd and kqueue.** Linux has `signalfd`: block the signals, and
read them from an fd as structs, no handler at all. The BSDs and the
Mac let `kqueue` report signals with `EVFILT_SIGNAL`. Both are cleaner
than a self-pipe, and neither is portable, so libraries like libuv
and Python's `asyncio` (`signal.set_wakeup_fd`, which is literally a
self-pipe) still use the trick.

**Crash handlers.** A `SIGSEGV` handler that prints a backtrace runs
in a process whose heap may be the thing that's broken. Good ones,
like Chromium's and the one in LLVM, use only safe calls, format
numbers by hand exactly as you did, and often run on an alternate
stack (`sigaltstack`), because the crash may have been a stack
overflow with no room left for the handler's frame.

**What C++ itself promises.** The C++ standard says a handler may only
touch lock-free atomics and `volatile sig_atomic_t`, plus a short
list of "plain lock-free" operations. In practice compilers allow what
POSIX allows, but `std::atomic<int>` with `is_always_lock_free` is the
portable way to share more than a flag.
