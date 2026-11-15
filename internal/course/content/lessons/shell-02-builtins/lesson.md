---
title: "cd, exit and other builtins"
kind: capstone
project: conch
program: conch
minutes: 45
run:
  interactive: true
deeper:
  - title: "Special built-in utilities, in the POSIX spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_15
  - title: "bash's builtins, documented"
    url: https://www.gnu.org/software/bash/manual/html_node/Shell-Builtin-Commands.html
---
Try `cd /tmp` in last lesson's conch, then `pwd`. You're still
where you were. `cd` ran as a program, in a child, and changed the
*child's* folder, which vanished with it a moment later. The current
folder is per process, and a child can't change its parent's.

So some commands have to run inside the shell itself: **builtins**.
Anything that changes the shell's own state must be one. That means
the current folder (`cd`), whether it keeps running (`exit`), and
its environment, which every later command inherits (`export`,
`unset`). Others, like `type`, are builtins because only the shell
knows the answer.

So before forking, conch looks the first word up in a table of
builtins. If it's there, conch calls it, in the shell, and what it
returns is `$?`. A table of names and functions (a `std::map` from
`std::string` to a function pointer, say) keeps that one lookup. These
six come first:

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
                search from lesson 6.1.3; a NAME with a / is found only if
                it's an executable file itself), or "conch: type: NAME:
                not found" on stderr with status 1
```

`exit` shouldn't leave on the spot, with `std::exit`: the shell has
things to do on the way out in later lessons, and `run_line` is also
called by the tests, which don't want to end. Let it mark the shell as
finished, with the code to leave with, and have `main` stop the loop.
Messages go to stderr and output to stdout, through `std::cerr` and
`std::cout`. A builtin's output is in `std::cout`'s buffer until it's
flushed, so flush after each builtin: the next command writes to the
same stdout, straight to the fd.

## Contract

Nothing new. The tests run scripts through your program, and `cd` in a
`Shell` of their own.

## Assignment

- [ ] `cd` and `pwd` work on the shell's own folder, including `cd` with no argument, `cd -`, and setting `PWD` and `OLDPWD`
- [ ] `exit` ends conch with no number, a number, or a bad number (2); with too many it complains and conch carries on
- [ ] `export` and `unset` change the environment that later commands get, and `export` rejects bad names
- [ ] `type` says which of the six names are builtins, where a program is, or that a name isn't found
