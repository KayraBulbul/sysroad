**How bash reads its input.** When stdin is a file or a pipe, a shell
mustn't read ahead: the commands it runs share that input, and
`head -n 1` on the next line expects to get the line after it. bash
reads a pipe one byte at a time, and for a file it reads a block and
then `lseek`s back to the end of the line it used. conch's `main.cpp`
reads one byte at a time, which is slow but always right.

**Interactive or not.** A shell decides whether it's interactive with
`isatty`: a terminal on stdin (and stderr, in bash) means a person is
typing. Interactive shells print prompts, ignore Ctrl-C, run job
control and read `~/.bashrc`; scripts get none of that. `bash -i`
forces it.

**The fork-exec cost of a shell script.** Every external command in a
script is a fork and an exec, roughly 0.5-1 ms each. That's why
builtins exist even for things like `echo`, `test` and `printf` (bash
has its own of each), and why a loop that runs `expr` a million times
takes minutes while the same loop with `$((...))` takes a second.

**Not found, or can't run.** `execvp` fails with `ENOENT` when no
folder on `PATH` has the program, and `EACCES` when it exists but
isn't executable. POSIX gives them different statuses, 127 and 126,
so a script can tell "missing" from "broken". `command -v name` asks
the shell what it would run, without running it.
