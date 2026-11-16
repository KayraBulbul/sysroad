---
title: "fg, bg and stopped jobs"
kind: capstone
project: conch
program: conch
minutes: 40
run:
  interactive: true
deeper:
  - title: "GNU libc manual: continuing stopped jobs"
    url: https://www.gnu.org/software/libc/manual/html_node/Continuing-Stopped-Jobs.html
  - title: "bash manual: job control builtins"
    url: https://www.gnu.org/software/bash/manual/html_node/Job-Control-Builtins.html
---
A job Ctrl-Z stopped waits in the table, going nowhere. Two builtins
bring it back:

- **`fg [%n]`** brings the job to the foreground: the newest job, or
  job `n`. It takes the job out of the table, prints its text, hands it
  the terminal with the settings it had when it stopped, sends `SIGCONT`
  to its group, and waits for it as for any foreground job. It may stop
  again, and then goes back in the table under its old number.
- **`bg [%n]`** carries the job on in the background: `SIGCONT` to its
  group, mark it running, and print `[n] text &`.

With no job to pick, they say `conch: fg:` (or `bg:`) and why, with
status 1. Without a terminal there's no job control, and they say
`conch: fg: no job control` with status 1.

**Noticing stops in the background.** A background job can stop too:
`cat &` reads the terminal, which isn't its group's, and gets
`SIGTTIN`. So when checking on background jobs before each line, also
pass `WUNTRACED | WCONTINUED`: a job that stopped shows as `Stopped` in
`jobs`, and one continued from elsewhere (`kill -CONT`) as `Running`
again.

**Leaving with stopped jobs.** `exit` with stopped jobs in the table
says `conch: there are stopped jobs` and doesn't exit, the first time
(status 1). The second time it lets the shell go, after sending each
stopped job `SIGHUP` and then `SIGCONT`, so nothing is left stopped
with nobody to continue it.

## Contract

Nothing new.

## Assignment

- [ ] `fg` and `fg %n` print the job's text, give it the terminal and its own settings back, continue it and wait for it; if it stops again it keeps its number
- [ ] `bg` and `bg %n` continue a job in the background and print `[n] text &`
- [ ] A background job that stops, or is continued, shows as `Stopped` or `Running` in `jobs`
- [ ] `exit` with stopped jobs warns once and stays; the second `exit` hangs them up and leaves
- [ ] Without a terminal, or with no such job, `fg` and `bg` say so and return 1
