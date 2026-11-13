---
title: Process groups and sessions
minutes: 35
files: [job.cpp]
program: pgroup
run:
  args: []
deeper:
  - title: "CS:APP 8.5.2: sending signals, and process groups"
    url: https://csapp.cs.cmu.edu/
  - title: "credentials(7): process groups and sessions"
    url: https://man7.org/linux/man-pages/man7/credentials.7.html
  - title: "setpgid(2) man page"
    url: https://man7.org/linux/man-pages/man2/setpgid.2.html
---
Press Ctrl-C during `find / | grep x | head`, and all three stop. The
terminal didn't send `SIGINT` to three PIDs it had to know about. It
sent it to one **process group**.

Every process belongs to a process group, named by a number, the
**PGID**. Children start in their parent's group. A shell puts each
**job** (a pipeline, or a single command) in a group of its own, led
by its first process, so the group's ID is that process's PID. Then
a negative PID in `kill` means "the whole group":

```cpp
setpgid(pid, pgid);   // put pid in group pgid; setpgid(pid, 0) makes pid lead a new one
getpgid(pid);         // which group is it in?
kill(-pgid, SIGINT);  // everyone in the group
```

There's a race in setting it up. The parent wants every process in the
group before it does anything with the job, and the child wants to be
in it before it `exec`s (or the command might run, and read the
terminal, from the wrong group). Neither knows which runs first after
`fork`. So **both** call `setpgid(child, pgid)`: the child on itself
before `exec`, the parent right after `fork`. Whichever comes second
finds it already done. If the child has already `exec`'d, the
parent's call fails with `EACCES`, and that's fine: it's done.

Groups live in a **session**. A session is everything started from
one login or one terminal window, and the terminal belongs to it: it
is the session's **controlling terminal**. Your shell leads its
session (its PID is the session ID), and every job you run is a group
inside it. `setsid()` starts a new session with no terminal at all,
which is how daemons cut loose. Next lesson puts the terminal to work.

```text
session 4100 (controlling terminal /dev/pts/3)
├── group 4100: bash                          (the session leader)
├── group 5210: find / | grep x | head        (a job: three processes)
└── group 5300: sleep 100 &                   (a background job)
```

`launch` is your pipeline from 6.1.7 in a group of its own, without
the waiting, which moves to `wait_job`.

## Assignment

- [ ] `launch` starts the pipeline with every process in a new group led by the first command, calling `setpgid` in both the child and the parent
- [ ] `signal_job` signals the whole group with one `kill`, and throws if it fails
- [ ] `wait_job` waits for every process, in order
- [ ] `start_detached` starts a command in a new session with `setsid`
- [ ] Run `ps -o pid,pgid,sid,comm` and find your shell's session and its group
