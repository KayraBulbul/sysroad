**`#!` lines.** `execve` on a file starting with `#!` makes the kernel
run the interpreter named on that line instead, with the script's
path as an argument: `execve("./deploy.py")` becomes
`execve("/usr/bin/env", {"/usr/bin/env", "python3", "./deploy.py"})`.
So `env` is there to find `python3` through `PATH`, because the `#!`
line itself needs an absolute path. A file with no `#!` and no
recognisable format fails with `ENOEXEC`; `execvp` and shells then
run it with `/bin/sh` themselves, for old scripts that predate `#!`.

**What the kernel actually loads.** For an ELF executable (or Mach-O
on the Mac, as in phase 5), `execve` maps its segments into a fresh
address space, then reads the `PT_INTERP` segment and maps the dynamic
loader too, and starts *it*, not your `main`. The loader maps the
shared libraries, runs constructors, and only then calls `main`. The
top of the new stack holds `argc`, the `argv` pointers, the `envp`
pointers, the **auxiliary vector** (where the program was loaded, the
page size, a few random bytes for stack canaries) and then the
strings themselves. `LD_SHOW_AUXV=1 /bin/true` prints it on Linux.

**What survives an exec.** The PID, parent, fds (except those marked
close-on-exec, lesson 5), current folder, `umask`, resource limits,
and signals set to "ignore". Handlers don't survive, because the code
they pointed to is gone, so those signals go back to their default.
Memory, threads and `atexit` handlers are all gone.

**Argument size limits.** `argv` and the environment together have to
fit in a limit, which is why `rm *` in a folder of a million files
fails with "Argument list too long" (`E2BIG`). On Linux it's a
quarter of the stack limit, about 2 MB with the usual 8 MB stack;
`getconf ARG_MAX` prints it. That's what `xargs` is for: it splits a
long list into several runs.

**Go and Python.** `exec.Command("ls")` looks the name up with
`exec.LookPath` when you create it, exactly your `find_in_path`, then
forks and execs on `Start`. Python's `subprocess.run` does the same
in C (`_posixsubprocess`). Both build the C arrays before forking, for
the reason the last lesson's under the hood gave: after a fork in a
threaded program, the child mustn't call `malloc`.
