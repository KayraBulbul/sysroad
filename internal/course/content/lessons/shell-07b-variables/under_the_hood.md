**Why two kinds of variable.** The environment is copied into every
process the shell starts, so everything in it costs a little at every
`exec`, and leaks into programs that never asked for it. Shell
variables are the shell's own scratch space: loop counters, temporary
values, settings like `PS1`. `export` is the decision to hand one on.

**Special builtins.** POSIX calls `export`, `unset`, `exit`, `.`,
`eval`, `readonly`, `set`, `shift`, `trap` and a few others *special*
builtins, and says assignments in front of them stay, as if they were
on a line of their own. bash only follows that in POSIX mode; for the
rest it keeps the assignment for the builtin alone, which is what conch
does for the others.

**`env` and `X=1 cmd`.** `env X=1 cmd` does the same as `X=1 cmd`, as
a program: it sets the variable in its own environment and `exec`s the
command. It's how you write the same thing where there's no shell, in
a crontab line or a `#!` line.

**The environment is just memory.** `environ` is an array of
`NAME=value` strings in the process's own memory, laid out by the
kernel at `exec`, after the arguments. `setenv` edits that copy, which
is why a child can never change its parent's environment, the same
reason `cd` must be a builtin.
