**Why `-g -O0`.** `-g` writes DWARF debug info into the binary (a map
from machine code addresses back to files, lines and variable names). With
optimisation on, variables live in registers or vanish, lines get
reordered, and the debugger shows `<optimized out>`. The lessons build
with `-O0` for the same reason.

**What a breakpoint is.** gdb overwrites the first byte of the
instruction at that address with `int3` (0xCC on x86-64) or `brk` on
arm64. When the CPU runs it, the kernel stops the process and tells the
debugger, through the `ptrace` system call on Linux. Phase 6 uses
`ptrace`'s relative, signals.

**Why the program's own output vanished.** Run `./drill | cat` and the
`walk(...)` lines never appear. stdout to a pipe is *fully buffered*: the
text sits in a buffer until it's full or the program exits normally, and a
crash means it never gets flushed. To a terminal it's line-buffered, so
you see it. That's why errors go to stderr, which is unbuffered.

**Core dumps.** Your PC uses systemd-coredump: `coredumpctl list` shows
recent crashes and `coredumpctl gdb` opens the last one in gdb, at the
moment it died. On macOS, crash reports land in
`~/Library/Logs/DiagnosticReports`.
