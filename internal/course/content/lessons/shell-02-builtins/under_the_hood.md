**Why `cd` can't be a program.** Every process has its own current
folder, inherited at `fork`. A `/usr/bin/cd` exists on macOS (it's a
tiny script required by POSIX), and running it does exactly nothing
useful: it changes its own folder and exits. The same goes for
`export`: a child's environment is a copy, so `printenv` can't show
what a child set.

**Special builtins.** POSIX divides builtins into *special* ones
(`exit`, `export`, `set`, `.`, `eval` and a few more), whose variable
assignments stick and whose errors can end a script, and *regular*
ones (`cd`, `type`, `read`), which behave more like programs. That's
why `VAR=x export Y` and `VAR=x cd /` differ in bash's POSIX mode.

**Logical and physical paths.** bash's `pwd` prints `$PWD`, the path
you *typed* through symlinks, while `pwd -P` prints the real path from
`getcwd`. On a Mac `cd /tmp; pwd` prints `/tmp` in bash and
`/private/tmp` in conch, because `/tmp` is a symlink there. bash
tracks the logical path by editing `$PWD` as you `cd`, resolving `..`
by removing the last part rather than following the link.

**`exec`, the builtin.** `exec cmd` replaces the shell with `cmd`: no
fork at all, just `execvp`. Login scripts use it to swap one shell for
another, and wrapper scripts end with `exec "$@"` so the real program
gets the shell's PID and its signals.
