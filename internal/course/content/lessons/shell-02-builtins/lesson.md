---
title: "cd, exit and other builtins"
kind: capstone
program: conch
minutes: 40
files: [builtins.cpp]
run:
  interactive: true
deeper:
  - title: "Special built-in utilities, in the POSIX spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_15
  - title: "bash's builtins, documented"
    url: https://www.gnu.org/software/bash/manual/html_node/Shell-Builtin-Commands.html
---
Try `cd /tmp` in last milestone's conch, then `pwd`. You're still
where you were. `cd` ran as a program, in a child, and changed the
*child's* folder, which vanished with it a moment later. The current
folder is per process, and a child can't change its parent's.

So some commands have to run inside the shell itself: **builtins**.
Anything that changes the shell's own state must be one. That means
the current folder (`cd`), whether it keeps running (`exit`), and
its environment, which every later command inherits (`export`,
`unset`). Others, like `type`, are builtins because only the shell
knows the answer.

`exec.cpp` now checks `find_builtin` before forking: if it finds one,
it calls it in the shell and uses what it returns as `$?`. You write
the builtins and the table that `find_builtin` looks in:

```text
cd [DIR]        no DIR: $HOME; "-": $OLDPWD, and print where you land.
                Sets OLDPWD and PWD. Errors: "conch: cd: DIR: <strerror>", status 1
pwd             print getcwd()
exit [N]        leave with N (only its low 8 bits), or with $? if there's no N.
                Not a number: "conch: exit: N: numeric argument required", leave with 2.
                More than one N: complain, status 1, and stay
export NAME=VALUE... or NAME...
                setenv each; a NAME with no value and not set yet becomes "".
                A bad name (empty, starts with a digit, not letters, digits
                and _): complain, status 1
unset NAME...   unsetenv each
type NAME...    "NAME is a shell builtin", "NAME is /path/to/it" (the PATH
                search from lesson 6.1.3), or "conch: type: NAME: not found"
                on stderr with status 1
```

`exit` doesn't leave on the spot: it sets `sh.exiting` and
`sh.exit_code`, and `main` stops the loop. Messages go to stderr,
output to stdout, through `std::cout` and `std::cerr`.

## Assignment

- [ ] `cd` and `pwd`, including `cd` with no argument, `cd -`, and setting `PWD` and `OLDPWD`
- [ ] `exit`, with no number, a number, a bad number and too many
- [ ] `export` and `unset`
- [ ] `type`, and `find_builtin` returning each of the six
