**Why piping changes the output.** Run `./forks` in a terminal with
the first flush removed and it looks fine; pipe it through `cat` and
the first line appears twice. stdout is *line buffered* when it's a
terminal (flushed at every `\n`) and *fully buffered* otherwise
(flushed every 4-64 KB). So with a terminal the buffer was already
empty when you forked. Bugs like this pass every manual test and then
show up in the log file.

**How fork copies a process cheaply.** The kernel copies the page
tables, not the pages, and marks every writable page read-only in
both processes. The first write to a page faults; the kernel sees
it's a copy-on-write page, copies those 4 KB (16 KB on Apple
silicon), and lets the write go ahead. Forking a 10 GB process still
has to copy its page tables, around 20 MB, which is why forking a big
database or JVM takes milliseconds rather than microseconds.

**vfork and posix_spawn.** Most forks are followed at once by an
`exec` (next lesson), which throws the copy away. `vfork` skips the
copy: the child borrows the parent's memory, and the parent is paused
until the child `exec`s or `_exit`s. It's fast and fragile. The modern
answer is `posix_spawn`, which does fork-and-exec in one call and is
free to use `vfork` or Linux's `clone(CLONE_VM | CLONE_VFORK)`
inside. Go's `os/exec` and Python's `subprocess` use it when they can.

**Forking a threaded program.** Only the thread that called `fork`
exists in the child. If another thread held a lock, say the one inside
`malloc`, the child has the lock held and no thread to release it, and
its next `malloc` hangs forever. So after forking a threaded program,
the child may only call **async-signal-safe** functions (the same list
as signal handlers, later this chapter) until it `exec`s. Python
prints a DeprecationWarning when you `os.fork()` with threads running.

**The memory-overcommit gamble.** If a 10 GB process forks, does the
system need 20 GB? Not unless both write to every page. Linux bets
they won't and lets the fork succeed (overcommit), which is why it
needs the OOM killer for when the bet loses. "A fork() in the road",
in the further reading, is a Microsoft Research paper arguing fork is
a design mistake, from the people who had to support it on Windows.
