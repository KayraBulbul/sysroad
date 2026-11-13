**The kernel's side.** Linux keeps each process in a `task_struct`, a
few kilobytes of state: the PID, a pointer to the parent, a pointer to
the memory map (`mm_struct`), one to the fd table (`files_struct`),
the current directory (`fs_struct`), credentials and signal handlers.
Threads are tasks too, sharing the same memory map and fd table. That's
all a thread is, to Linux.

**Where the environment lives.** The kernel copies the program's
arguments and environment to the top of the new stack when it starts
a program, and `environ` points there. `setenv` can't grow that block,
so it allocates a new array on the heap and repoints `environ`. You
can see a running program's original environment in
`/proc/<pid>/environ`, with null bytes between the entries
(`tr '\0' '\n' < /proc/self/environ`), but not later `setenv`s.

**PID 1.** Every process has a parent, back to PID 1: `systemd` on
your Linux machine, `launchd` on the Mac. The kernel starts it, and it
starts everything else. `pstree` (Linux) or `pstree` from Homebrew
shows the whole family tree. When a parent exits first, its children
get a new parent: PID 1, or the nearest ancestor that asked to be a
"subreaper" (systemd's user manager does).

**PIDs get reused.** A PID only names a process while it exists.
Linux hands them out in increasing order up to
`/proc/sys/kernel/pid_max` (4,194,304 on 64-bit systems) and then
wraps around. Code that stores a PID and signals it much later can hit
an unrelated process. Linux's `pidfd_open` gives you an fd that
refers to exactly one process, which fixes that.

**The Mac has no /proc.** Its tools ask the kernel through `sysctl`
and the `libproc` library instead: `proc_pidinfo` and
`proc_pidpath`, which `ps`, `lsof` and Activity Monitor use. `/dev/fd`
works on both because it's in POSIX's spirit, if not its letter.
