---
title: History
kind: capstone
project: conch
program: conch
minutes: 35
run:
  interactive: true
deeper:
  - title: "bash manual: Bash History Facilities"
    url: https://www.gnu.org/software/bash/manual/html_node/Bash-History-Facilities.html
  - title: "rename(2) (man7.org)"
    url: https://man7.org/linux/man-pages/man2/rename.2.html
---
A shell remembers what you typed. This lesson conch keeps a **history**
of its lines, shows it with a builtin, and keeps it in a file between
sessions; next lesson, Up and Down walk through it.

Every line conch reads goes into the history, before it runs, except a
line that's blank (only spaces and tabs) or the same as the line before
it. The history keeps the newest 1000 lines, oldest first. A comment is
a line like any other.

**`history`** prints them numbered from 1, the number in five columns
then two spaces, as bash does (`printf("%5zu  %s\n", ...)`).
`history N` prints only the last N, still with their numbers. A count
that isn't a number says `conch: history: N: numeric argument
required` with status 1.

**Between sessions**, when conch is talking to a terminal, it reads
`~/.conch_history` (`$HOME/.conch_history`) before the first prompt,
each line through the same rules, and writes the history back when it
ends, by `exit` or at the end of the input. A script doesn't read or
write it. Save safely, as kite did: write a temporary file next to it
and `rename` it over the old one, so a crash never leaves half a
history. History is private (people paste passwords at prompts by
mistake), so the file is readable by its owner only, mode `0600`,
whatever an older file had.

## Contract

Nothing new.

## Assignment

- [ ] Each line read goes into the history, except a blank line or a repeat of the last one, keeping the newest 1000
- [ ] `history` lists them numbered from 1 (`%5zu` then two spaces), and `history N` the last N
- [ ] On a terminal, conch reads `~/.conch_history` at the start and writes it at the end, through a temporary file and `rename`, with mode `0600`; a script leaves it alone
