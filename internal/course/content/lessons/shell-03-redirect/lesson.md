---
title: Redirections
kind: capstone
project: conch
program: conch
minutes: 45
run:
  interactive: true
deeper:
  - title: "Redirection, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_07
  - title: "open(2) (man7.org)"
    url: https://man7.org/linux/man-pages/man2/open.2.html
---
Time for `>`. A line is no longer just words: it has **operators**
too. This lesson, the redirections:

```text
<  FILE      read fd 0 from FILE            <&  N     fd 0 becomes a copy of fd N
>  FILE      write fd 1 to FILE (emptied)   >&  N     fd 1 becomes a copy of fd N
>> FILE      append fd 1 to FILE
```

Each can have an fd number written right before it: `2> err.txt`,
`2>&1`, `3< in.txt`. Without one, it's fd 0 for `<` and `<&`, and fd 1
for the rest.

**Tokens.** First the line is split into **tokens**: words, and
operators with their fd number. Operators don't need spaces around
them, so `echo hi>out.txt` is three tokens, and a word ends where an
operator starts. Digits right before a `<` or `>` are its fd number,
not a word: `2>&1` is one operator and a word, while `a2>f` is the
word `a2` and `>`. More than 9 digits is a syntax error, rather than a
number that doesn't fit in an `int`.

```text
sort -r <in.txt 2>>log
→ word "sort", word "-r", op "<", word "in.txt", op ">>" (fd 2), word "log"
```

**Parsing** turns the tokens into a command: its words, and its
redirections in order, wherever they are in the line (`> f echo hi`
works). Each operator takes the next token as its target, which must
be a word: a file name, or for `<&` and `>&` an fd number. Anything
else is a **syntax error**: conch prints `conch: syntax error:` and
what's wrong to stderr, and the line's status is 2.

**Applying** them is lesson 6.1.5: in the child, after the fork and
before the `exec`, for each redirection in order, `open` the file and
`dup2` it onto the fd, or just `dup2` for `<&` and `>&`. Order matters:
`cmd > f 2>&1` sends both to `f`, while `cmd 2>&1 > f` sends stderr
where stdout *was*. When one fails, the child prints `conch:`, the file
(or the fd number, for `>&`), and the reason, then exits with 1. A
command with no words, just `> f`, still makes the file.

Builtins get their redirections next lesson: they run inside the shell,
which makes them harder. Until then, they ignore them.

## Contract

Nothing new.

## Assignment

- [ ] A line splits into words and the five redirection operators, with or without spaces, and with an fd number before an operator
- [ ] Each operator takes the word after it; a missing or wrong target is a syntax error, reported on stderr with status 2
- [ ] An external command's redirections apply in its child, in order, with the right default fds
- [ ] A redirection that fails prints `conch: FILE: reason` (or the fd number) and gives status 1, and `> FILE` alone makes the file
