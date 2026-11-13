---
title: wait and exit statuses
minutes: 35
files: [status.cpp]
program: retry
run:
  args: ["-n", "4", "sh", "-c", "echo rolling a die; test $(( $$ % 3 )) = 0"]
deeper:
  - title: "CS:APP 8.4.3: reaping child processes"
    url: https://csapp.cs.cmu.edu/
  - title: "waitpid(2) man page"
    url: https://man7.org/linux/man-pages/man2/waitpid.2.html
  - title: "Exit status, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_08_02
---
When a process ends, it doesn't vanish. The kernel keeps a small
record of how it ended until its parent asks, with `waitpid`. Until
then it's a **zombie**: dead, holding nothing but its PID and that
record. `ps` shows it as `Z` or `<defunct>`.

```cpp
int wstatus;
pid_t pid = waitpid(child, &wstatus, 0);   // blocks until it ends
```

`wstatus` packs several things into one int, and macros unpack it. A
process ends one of two ways:

- it **exited**, by returning from `main` or calling `exit`:
  `WIFEXITED(wstatus)` is true and `WEXITSTATUS(wstatus)` is the code,
  0 to 255
- it was **killed by a signal** (next chapter), like Ctrl-C's
  `SIGINT` or a segfault's `SIGSEGV`: `WIFSIGNALED(wstatus)` is true,
  `WTERMSIG(wstatus)` is the signal's number, and `WCOREDUMP(wstatus)`
  says whether it left a core dump

Shells fold both into one number, `$?`: the exit code, or 128 plus the
signal's number. That's why a Ctrl-C'd command reports 130 and a
`kill -9`'d one 137.

`waitpid`'s first argument picks whom to wait for: a PID, or -1 for
any child. With the `WNOHANG` option it doesn't block: it returns 0 if
none of those children have finished yet. A loop of
`waitpid(-1, &st, WNOHANG)` collects every finished child, and stops
at 0 (some are still running) or -1 with `errno == ECHILD` (there are
none). A shell does exactly that to notice its background jobs
ending.

`waitpid` can also fail with `EINTR` if a signal arrives while it's
blocked. That isn't an error; you just call it again.

Put `fork`, `exec` and `waitpid` together and you have `start` and
`wait_for`: `subprocess.Popen` and `.wait()`, or Go's `cmd.Start()`
and `cmd.Wait()`. The program this time is `retry`, which runs a
flaky command until it works.

## Assignment

- [ ] `start` builds its arguments, flushes, forks and `execvp`s; if that fails, the child prints why and `_exit`s with 127
- [ ] `decode` unpacks a `wstatus`, and `wait_for` waits for one child, retrying on `EINTR` and throwing on other errors
- [ ] `signal_name` names at least `SIGHUP INT QUIT ILL TRAP ABRT BUS FPE KILL USR1 SEGV USR2 PIPE ALRM TERM CHLD CONT STOP TSTP TTIN TTOU`, and `describe` and `shell_status` format a `Status`
- [ ] `reap_finished` collects every finished child with `WNOHANG`, and never blocks
