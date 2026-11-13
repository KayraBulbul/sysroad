---
title: Redirections
kind: capstone
program: conch
minutes: 45
files: [parser.cpp, redirect.cpp]
run:
  interactive: true
deeper:
  - title: "Redirection, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_07
  - title: "fcntl(2): F_DUPFD_CLOEXEC"
    url: https://man7.org/linux/man-pages/man2/fcntl.2.html
---
Time for `>`. The course's lexer now splits a line into **tokens**:
words, and the redirection operators `<`, `>`, `>>`, `<&` and `>&`,
each with the fd number written in front of it, if any. Operators
don't need spaces around them: `echo hi>out.txt` and `2>&1` both work.

```text
sort -r <in.txt 2>>log
→ Word "sort", Word "-r", Op "<", Word "in.txt", Op ">>" (fd 2), Word "log"
```

**Parsing** turns tokens into a `Command`: its words in `argv`, and
its `redirects` in order. Each operator takes the next token as its
target, which must be a word: a file name, or for `<&` and `>&` an fd
number. Anything else is a `SyntaxError`, which `run_line` reports as
`conch: syntax error: ...` with status 2. The fd defaults to 0 for
`<` and `<&`, and 1 for the rest.

**Applying** them is lesson 6.1.5: for each in order, `open` and
`dup2`, or just `dup2` for `<&` and `>&`, throwing
`std::system_error` named after the file (or fd) when it fails. An
external command applies them in the child after the fork, so a
failure there ends the child with status 1.

A builtin is harder. `type cd > t.txt` runs *in the shell*, so the
redirection changes the shell's own stdout, and it has to be put back
afterwards or every later command writes to `t.txt`. That's
`SavedFds`: before applying, it copies each fd the redirections will
touch somewhere out of the way, and its destructor puts them back.
Copy with `fcntl(fd, F_DUPFD_CLOEXEC, 10)`: the copy lands at fd 10 or
above, away from fds a user might name, and it's close-on-exec, so no
command the shell runs later inherits it. If an fd wasn't open at all
(`fcntl` fails with `EBADF`), remember that, and close it again at the
end. Any other failure is different: with an fd limit of 10 or less,
there's no fd 10 to be had, so try again with a minimum just above every
fd the redirections name (so a copy can't stand in for one of them),
and if there's still no room, throw, before anything has been changed.

## Assignment

- [ ] `parse_command` collects words and redirections, with the right default fds, and throws `SyntaxError` for a missing or wrong target
- [ ] `apply_redirects` applies them in order, throwing `std::system_error` named after the file or fd
- [ ] `SavedFds` saves each affected fd once, as a close-on-exec copy at 10 or above (or anywhere, if the limit is lower), throws if it can't, and restores (or closes) them in its destructor
- [ ] Run conch and try `ls > files.txt`, `cat < files.txt`, and `cat nothing 2>&1 > /dev/null`
