---
title: Redirecting a builtin
kind: capstone
project: conch
program: conch
minutes: 35
run:
  interactive: true
deeper:
  - title: "fcntl(2): F_DUPFD_CLOEXEC (man7.org)"
    url: https://man7.org/linux/man-pages/man2/fcntl.2.html
  - title: "Redirection, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_07
---
`type cd > t.txt` runs *in the shell*, so its redirection changes the
shell's own stdout. Applied the way an external command's are, it stays
applied: every later command, and the prompt, would write to `t.txt`.
So a builtin's redirections have to be undone when it's done.

The way to undo a `dup2` is to have kept a copy. Before applying, copy
each fd the redirections will touch somewhere out of the way; after the
builtin, `dup2` each copy back onto its fd and close the copy. That's a
job for RAII again: a class that saves in its constructor and restores
in its destructor, so every way out of running the builtin, including
an exception from a failed redirection, puts the shell back as it was.
The details are where it gets interesting:

- **Each fd once.** `> a.txt > b.txt` touches fd 1 twice. Save it the
  first time only, or the second save copies `a.txt`.
- **Out of the way.** Copy with `fcntl(fd, F_DUPFD_CLOEXEC, n)`: the
  copy lands at fd `n` or above, and it's close-on-exec, so nothing the
  shell runs inherits it. Make `n` at least 10, out of the way of the
  fds scripts name, and above every fd the redirections name, so a copy
  can never stand in for one of them: `pwd 1>&10` with fd 10 closed
  must still fail.
- **Closed fds.** If an fd wasn't open at all (`fcntl` fails with
  `EBADF`), remember that, and close it again at the end.
- **No room.** Any other failure is different: with a limit of 10 open
  files or fewer, there's no fd 10 to be had. Then try again with a
  minimum just above every fd the redirections name, and if there's
  still no room, give up before changing anything: the builtin doesn't
  run, conch says why, and the status is 1.

A redirection that fails, as for an external command, prints
`conch: FILE: reason` and gives status 1, and the builtin doesn't run.
Flush the builtin's output before putting the fds back, or it comes out
on the restored stdout later.

## Contract

Nothing new. Some tests run builtins in a `Shell` of their own and
check that the test process's fds are as they were afterwards.

## Assignment

- [ ] A builtin's redirections apply while it runs, and afterwards every fd is as it was, including ones that were closed
- [ ] An fd redirected twice is restored to what it was before the first
- [ ] Saved copies are close-on-exec, at 10 or above and above every fd the redirections name, or just above those when the limit on open files is 10 or less
- [ ] A redirection that fails, or an fd that can't be saved, stops the builtin with a message and status 1, leaving the shell's fds alone
