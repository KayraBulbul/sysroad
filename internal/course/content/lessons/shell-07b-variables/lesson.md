---
title: Shell variables
kind: capstone
project: conch
program: conch
minutes: 45
run:
  interactive: true
deeper:
  - title: "Shell variables, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_05
  - title: "Simple commands: where the assignments go (POSIX)"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_01
---
Until now every variable was an environment variable, set with
`export`. Shells have their own variables too, which commands don't
see unless they're exported.

**Assignments.** A word written `NAME=value`, with a valid name before
the `=` and nothing of the name quoted, is an **assignment** when it
comes before a command's first word: `X=1`, `A_B='x y'`. After the
first word it's an ordinary word (`echo A=1` prints `A=1`), and
`'A'=1` or `1A=x` aren't assignments at all. The lexer can mark them,
since it sees the quotes; the parser sets them aside from the words.
The value is expanded like any word, when the command runs, and each
assignment sees the ones before it on its command: `A=1 B=$A` gives
`B` 1.

**What an assignment does** depends on what follows it:

- **Nothing** (`X=1` alone): it sets a **shell variable**, kept in the
  shell, which `$X` sees and commands don't. If `X` is already
  exported, it changes the environment variable instead, which stays
  exported.
- **A command** (`X=1 cmd`): `X` is in `cmd`'s environment only, set
  in its child before the `exec`, and the shell's own `X` is untouched.
- **A builtin** (`HOME=/ cd`): as in bash, it lasts as long as the
  builtin runs, then the variable goes back to what it was. Except
  that `export`, `unset` and `exit` keep it (`X=1 export X`), and a
  change the builtin makes itself stays: `cd` sets `PWD`, and that
  isn't undone even when it's the variable assigned in front.

`$NAME` now looks for a shell variable first, then the environment.
`export NAME` moves a shell variable into the environment (or makes an
empty one, as before), and `export NAME=value` sets and exports.
`unset` removes a variable from both.

## Contract

Nothing new.

## Assignment

- [ ] `NAME=value` before a command's words is an assignment, only with an unquoted, valid name; its value is expanded when it runs, seeing earlier assignments
- [ ] Alone, it sets a shell variable that `$NAME` sees and commands don't, or changes an exported one
- [ ] Before a command, it's in that command's environment only; before a builtin, it lasts while the builtin runs, except for `export`, `unset` and `exit`, and a variable the builtin sets itself stays
- [ ] `export` moves a shell variable into the environment, and `unset` removes it from both
