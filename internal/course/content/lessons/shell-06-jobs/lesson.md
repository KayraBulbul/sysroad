---
title: "fg, bg and Ctrl-Z"
kind: capstone
program: conch
minutes: 50
files: [jobs.cpp]
run:
  interactive: true
deeper:
  - title: "GNU libc manual: implementing a job control shell"
    url: https://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html
  - title: "CS:APP shell lab (tsh): the same milestone, in C"
    url: https://csapp.cs.cmu.edu/3e/labs.html
---
Now conch gets what makes a shell feel like one: Ctrl-C stops the
command, not the shell; Ctrl-Z pauses it; `fg` and `bg` bring it back.
This is lesson 6.2.6 inside conch.

When `main` sees a terminal, it sets `sh.interactive`, points `sh.tty`
at fd 0 and calls your **`init_job_control`**: wait until its group is
in the foreground (`conch &` must not steal the terminal), ignore
`SIGINT SIGQUIT SIGTSTP SIGTTIN SIGTTOU`, lead a process group, take
the terminal. From
then on, `launch_job` (the course's) puts each job in a group of its
own, `job.pgid`, gives a foreground job the terminal from both sides
of the fork, and sets the signals back to their defaults in each
child. Without a terminal, none of this happens, and `fg` and `bg`
say `conch: fg: no job control` and return 1.

Your part, in `jobs.cpp`:

- **`run_foreground`** gives the job the terminal (when interactive),
  waits for its processes with `WUNTRACED`, then takes the terminal
  back with the shell's own terminal modes, as in 6.2.6. Wait for the
  PIDs in order rather than for the group, since without job control  there's no group. If a process stops, the job is stopped: it goes in
  the table with a number, `\n[n] Stopped  text` is printed, and `$?` is
  128 plus the signal that stopped it. Save the terminal modes it left
  (`tcgetattr` into `job.modes`) before putting the shell's back: a
  stopped `vim` is in raw mode, and wants to be again when it resumes.
- **`notify_jobs`** also passes `WUNTRACED | WCONTINUED`, so a
  background job that stops (say, `cat &` reading the terminal, which
  gets `SIGTTIN`) or is continued from elsewhere is noticed. `jobs` now
  says `Stopped` or `Running`.
- **`fg [%n]`** (the newest job if no `%n`) takes the job out of the
  table, prints its text, hands it the terminal and its saved modes,
  sends `SIGCONT` to its group and waits as above. It may stop again,
  and go back in the table under its old number. `wait %n` on a stopped
  job returns 128 plus its stop signal at once: it can't end until it's
  continued.
- **`bg [%n]`** sends `SIGCONT` to the group, marks it running and
  prints `[n] text &`.
- **`may_exit`**: `exit` with stopped jobs says `conch: there are
  stopped jobs` and doesn't exit, once. The second time it sends the
  stopped jobs `SIGHUP` and `SIGCONT` and lets the shell go.

## Assignment

- [ ] `init_job_control`, and a `run_foreground` that hands over the terminal, waits with `WUNTRACED`, and takes the terminal and its modes back
- [ ] A job that stops goes in the table, reported as `[n] Stopped  text`, with `$?` 128 plus the signal
- [ ] `notify_jobs` notices stops and continues; `jobs` shows `Stopped` and `Running`
- [ ] `fg` and `bg` with or without `%n`, and `may_exit`'s one warning
- [ ] Run conch: `sleep 30`, Ctrl-Z, `bg`, `jobs`, `fg`, Ctrl-C
