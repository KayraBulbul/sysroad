**The CS:APP version.** In their shell lab, `addjob` and `deletejob`
are called from `main` and from the `SIGCHLD` handler, on one shared
array. Their fix is to block *all* signals while either runs. Yours
moves the bookkeeping out of the handler entirely, which is how real
programs do it: the handler records the bare facts, and everything
else happens in normal code.

**Automatic reaping.** Setting `SIGCHLD` to `SIG_IGN` (or using
`SA_NOCLDWAIT`) tells the kernel to reap children itself. No zombies,
but `waitpid` then blocks until *all* children are gone and fails with
`ECHILD`, so you learn nothing about how they ended. Handy for a
server that forks fire-and-forget workers.

**Who else is reaping?** `waitpid(-1, ...)` reaps *any* child,
including ones a library started: `system()`, `popen()`, a logging
helper. If your handler reaps them first, their own `waitpid` fails
with `ECHILD`. glibc's `system` blocks `SIGCHLD` while it runs for
exactly this reason. Programs that care reap only PIDs they know, with
`waitpid(pid, ..., WNOHANG)`, or on Linux use pidfds.

**SIGCHLD carries details.** With `SA_SIGINFO`, the handler gets a
`siginfo_t` holding the child's PID, `si_code` (`CLD_EXITED`,
`CLD_KILLED`, `CLD_STOPPED`, `CLD_CONTINUED`) and its status. But since
signals don't queue, you still need the loop: the info describes one
child, and others may have ended too.

**Zombies at scale.** Containers often run a program as PID 1 that
was never meant to be one. Anything orphaned inside the container is
re-parented to it, and if it doesn't reap, zombies pile up. That's
why `docker run --init` and `tini` exist: a tiny PID 1 whose whole job
is this lesson's handler.
