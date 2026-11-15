---
title: Background jobs
kind: capstone
project: conch
program: conch
minutes: 55
run:
  interactive: true
deeper:
  - title: "Asynchronous lists, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_03_02
  - title: "bash manual: job control basics"
    url: https://www.gnu.org/software/bash/manual/html_node/Job-Control-Basics.html
---
End a pipeline with `&` and the shell doesn't wait for it: it starts
the pipeline and goes straight on to the next. That pipeline is a
**background job**, and the shell keeps a table of them.

**`&`** is one more operator. It ends a pipeline the way `;` does, and
marks it to run in the background: `sleep 1 & echo now` starts the
sleep and runs the echo at once. Each job remembers its **text**, its
tokens as they were written separated by single spaces (`true | sleep
1`), for the messages below.

**Starting and waiting, apart.** Last lesson's pipeline started its
commands and waited for them in one go. Split those in two: starting
gives back a **job** (the PIDs, in order, with room for how each one
ended), and then either

- **in the foreground**, wait for every process and use the last one's
  status, as before; or
- **in the background**, give the job a number, one more than the
  highest in the table (1 when it's empty), print `[n] pid` to stderr
  with the last command's PID, as bash does, keep the job in the table,
  and give the line status 0.

A background job's first command reads `/dev/null`, not the shell's
input: otherwise `cat &` in a script would eat the rest of the script.
A builtin in the background runs in a child, as in a pipeline.

**Noticing the end.** Nobody waits for a background job, so the shell
has to notice when one ends. Before reading each line, check each
process that hasn't ended with `waitpid(pid, &st, WNOHANG)`, which
returns at once. A job whose processes have all ended is reported on
stdout and dropped from the table:

```text
[1] Done  sleep 1             (the last command exited 0)
[2] Exit 3  make              (it exited with 3)
[3] SIGTERM  sleep 100        (it was killed by a signal)
```

Two builtins finish it. **`jobs`** notices first, then lists each job as
`[n] Running  text`. **`wait`** with no argument waits for every job,
drops them without a notice and returns 0. `wait %n` waits for that
job, drops it and returns its status, or says
`conch: wait: %n: no such job` and returns 127.

## Contract

Nothing new.

## Assignment

- [ ] `&` runs a pipeline in the background: `[n] pid` on stderr, status 0, and the shell carries on at once
- [ ] A background job's first command reads `/dev/null`
- [ ] Before each line, finished jobs are reported once (`Done`, `Exit N` or the signal's name, then the text), reaped and dropped
- [ ] `jobs` lists the running jobs; `wait` waits for all of them, or for `%n` and returns its status
- [ ] Job numbers are one more than the highest in the table
