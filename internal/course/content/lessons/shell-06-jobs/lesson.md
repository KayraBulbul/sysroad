---
title: "Ctrl-C, Ctrl-Z and process groups"
kind: capstone
project: conch
program: conch
minutes: 50
run:
  interactive: true
deeper:
  - title: "GNU libc manual: implementing a job control shell"
    url: https://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html
  - title: "CS:APP shell lab (tsh): the same milestone, in C"
    url: https://csapp.cs.cmu.edu/3e/labs.html
---
Now conch gets what makes a shell feel like one: Ctrl-C ends the
command, not the shell, and Ctrl-Z pauses it. This is lesson 6.2.6
inside conch. (Bringing a paused job back with `fg` and `bg` is next
lesson.)

All of this is for a person at a terminal, so it happens only when
conch's input is one. Then, before the first prompt, conch sets itself
up for **job control**:

- wait until its group is in the terminal's foreground, sending its
  group `SIGTTIN` until it is (`conch &` from another shell must not
  steal the terminal);
- ignore `SIGINT`, `SIGQUIT`, `SIGTSTP`, `SIGTTIN` and `SIGTTOU`, so
  the keyboard's signals don't reach it;
- lead a process group of its own, and take the terminal.

**Every job in a group of its own.** When starting a job, put all its
processes in one new process group, led by the first: `setpgid` in
both the child and the parent, since either may run first. A
foreground job also gets the terminal, `tcsetpgrp`, from both sides
for the same reason. In each child, set those five signals back to
their defaults before anything else, or they'd be ignored by the
command too (ignored signals stay ignored across `exec`). Now Ctrl-C
goes to the job's group, and the shell, in another group, doesn't hear
it. A background job, in a group without the terminal, is stopped by
`SIGTTIN` if it tries to read the terminal, so it no longer needs
`/dev/null` when there's job control.

**Waiting for a foreground job** changes too. Wait with `WUNTRACED`,
for the PIDs in order (without job control there's no group to wait
for), so a process that **stops** is noticed: then the whole job is
stopped. When the waiting is over, take the terminal back
(`tcsetpgrp` to the shell's own group) and put back the terminal
settings the shell had before the job, which a program like `stty` or
`vim` may have changed. Save the settings a stopped job leaves
(`tcgetattr`) first: a stopped `vim` is in raw mode, and wants to be
again when it carries on.

**A stopped job** goes in the job table, reported as
`\n[n] Stopped  text` on stdout (the newline because Ctrl-Z leaves the
cursor after `^Z`), and the line's status is 128 plus the signal that
stopped it. `jobs` now says `Stopped` or `Running`. `wait %n` on a
stopped job returns 128 plus its stop signal at once, since it can't
end until it's continued; plain `wait` skips stopped jobs. A job killed
by Ctrl-C has status 130, and leaves the cursor after `^C`, so print a
newline for the prompt.

## Contract

Nothing new. The tests run conch on a terminal of their own and press
Ctrl-C and Ctrl-Z.

## Assignment

- [ ] On a terminal, conch waits to be in the foreground, ignores the keyboard's signals and leads its own group
- [ ] Each job runs in a process group of its own, a foreground job has the terminal, and its processes get the default signal handling back
- [ ] Ctrl-C ends the foreground job and not the shell, with `$?` 130
- [ ] Ctrl-Z stops the foreground job: `[n] Stopped  text`, `$?` 128 plus the signal, and `jobs` lists it as `Stopped`
- [ ] After every foreground job the shell has the terminal back, with its own settings
