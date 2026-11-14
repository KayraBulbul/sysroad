---
title: Background jobs
base: shell-04-pipes
kind: capstone
program: conch
minutes: 45
files: [jobs.cpp]
run:
  interactive: true
deeper:
  - title: "Asynchronous lists, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_03_02
  - title: "bash manual: job control basics"
    url: https://www.gnu.org/software/bash/manual/html_node/Job-Control-Basics.html
---
End a pipeline with `&` and the shell doesn't wait for it: it starts
the job and goes straight back for the next line. That pipeline is a
**background job**, and the shell keeps a table of them.

The course has done the plumbing. The lexer knows `&`, the parser
marks the pipeline `background` (and keeps its words as `text`), and
`run_pipeline` now starts every external pipeline with `launch_job`,
which forks the commands with their pipes and returns a `Job`: its
PIDs, and room for how each ends. Then it calls one of yours:

- **`run_foreground`**: wait for every process in the job and return
  the last one's status, as `run_pipeline` did last milestone.
- **`start_background`**: give the job the next number (one more than
  the highest in the table, so 1 when it's empty), print `[n] pid` to
  stderr with the last command's PID, as bash does, add it to
  `sh.jobs` and return 0.

Nobody waits for a background job, so the shell has to notice when one
ends. Before reading each line, `main` calls your **`notify_jobs`**:
for each process that hasn't ended, `waitpid(pid, &st, WNOHANG)`, which
returns at once. A job whose processes have all ended is reported on
stdout and dropped from the table:

```text
[1] Done  sleep 1             (last command exited 0)
[2] Exit 3  make              (exited with 3)
[3] SIGTERM  sleep 100        (killed by a signal)
```

Two builtins finish it. `jobs` (notify first, then list
`[n] Running  text`), and `wait`: with no argument it waits for every
job, drops them silently and returns 0. With `%n` it waits for that job,
drops it and returns its status, or says `conch: wait: %n: no such job`
and returns 127.

One more rule: a background job's first command reads `/dev/null`, not
the shell's input. Otherwise `cat &` in a script would eat the rest
of the script. `launch_job` does that for you.

## Assignment

- [ ] `start_background` numbers the job, prints `[n] pid` to stderr and keeps it; `run_foreground` waits and returns the last status
- [ ] `notify_jobs` reaps without blocking, reports each finished job once (`Done`, `Exit N` or the signal's name) and drops it
- [ ] `jobs` lists the running jobs; `wait` waits for all of them, or for `%n` and returns its status
- [ ] Run `sleep 3 &`, `jobs`, and press Enter until it's done
