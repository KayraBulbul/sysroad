---
title: The terminal and job control
minutes: 45
files: [jobctl.cpp]
program: jobrun
run:
  args: ["sh", "-c", "echo 'press Ctrl-Z to stop me, Ctrl-C to end me'; sleep 5; echo finished"]
  interactive: true
deeper:
  - title: "The TTY demystified (Linus Åkesson)"
    url: https://www.linusakesson.net/programming/tty/
  - title: "GNU libc manual: implementing a job control shell"
    url: https://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html
  - title: "tcsetpgrp(3) man page"
    url: https://man7.org/linux/man-pages/man3/tcsetpgrp.3.html
---
A terminal has exactly one **foreground process group**. Only that
group can read from it, and the keys that make signals go to that
group alone:

```text
Ctrl-C   SIGINT    end it
Ctrl-\   SIGQUIT   end it, with a core dump
Ctrl-Z   SIGTSTP   stop it (pause, until SIGCONT)
```

That's how Ctrl-C reaches `vim` and not the shell behind it: when the
shell runs a command in the foreground, it gives the terminal to the
job's group, waits, and takes the terminal back.

```cpp
tcsetpgrp(tty, job_pgid);       // the job is in the foreground now
waitpid(-job_pgid, &st, WUNTRACED);   // WUNTRACED: tell me if it stops, too
tcsetpgrp(tty, getpgrp());      // back to the shell
```

Everything else in the session is in the **background**. A background
process that tries to read the terminal gets **`SIGTTIN`**, which
stops it, and one that tries to change the terminal's settings gets
**`SIGTTOU`**. That includes calling `tcsetpgrp`, which is awkward
for a shell, so shells ignore `SIGTTOU`. They ignore `SIGINT`,
`SIGQUIT` and `SIGTSTP` too: Ctrl-C at the prompt shouldn't end your
shell. But ignoring `SIGTTOU` also lets a shell that was started in the
background (`bash &`) grab the terminal from the shell that started it.
So before anything else, a shell checks: while `tcgetpgrp(tty)` isn't
its own group, it sends its group `SIGTTIN`, which stops it until
someone brings it to the foreground.

Ignored signals stay ignored across `exec` (lesson 6.1.6), so each
child must set those signals back to `SIG_DFL` before it runs its
command. Otherwise Ctrl-C would do nothing to it.

A stopped process hasn't ended, so plain `waitpid` doesn't return for
it. With `WUNTRACED` it does: `WIFSTOPPED(st)` is true and
`WSTOPSIG(st)` says which signal stopped it. A shell then prints
`[1]+ Stopped`, and the job waits until `fg` or `bg` sends **`SIGCONT`**
to its group: in the foreground (terminal first, then `SIGCONT`, then
wait again) or in the background (just `SIGCONT`).

Two more details make it solid:

- **Waiting on the group.** `waitpid(-pgid, ...)` waits for any process
  in the group, so a pipeline is waited for in whatever order its
  processes end or stop.
- **Terminal modes.** A program like `vim` switches the terminal to raw
  mode (phase 3) and might be killed before it switches back. Save the
  shell's modes with `tcgetattr` before the job runs and restore them
  with `tcsetattr` once you have the terminal back.

The tests run your code on a **pseudo-terminal**, a terminal made in
software, the same kind your terminal emulator gives you. They type
Ctrl-Z and Ctrl-C into it as you would.

## Assignment

- [ ] `init_job_control` first waits until its group is in the foreground (sending itself `SIGTTIN` until it is), then ignores `SIGINT SIGQUIT SIGTSTP SIGTTIN SIGTTOU`, leads its own group if it doesn't already, and takes the terminal
- [ ] `launch_job` puts the job in a new group, gives a foreground job the terminal from both the child and the parent, and sets the job signals back to `SIG_DFL` in each child
- [ ] `wait_for_job` waits on the group with `WUNTRACED` until every process has ended or one has stopped
- [ ] `wait_foreground` and `continue_job` hand over the terminal and take it back with the shell's own modes; `continue_job` sends `SIGCONT` to the group
- [ ] Run `jobrun`, press Ctrl-Z, then let it carry on
