**When bash reports.** An interactive bash reports finished jobs just
before the next prompt (or at once with `set -b`). It notices them
through `SIGCHLD`, with a handler much like lesson 6.2.4's, and checks
its table between commands. A non-interactive bash doesn't report at
all; conch reports anyway, so you can see it working.

**`$!` and `wait $pid`.** `$!` is the PID of the last background job
(conch gets it in milestone 7), and `wait $!` waits for that one. The
pattern `cmd & pid=$!; ...; wait $pid` is how scripts run things in
parallel. `wait -n` (bash) waits for whichever job ends first.

**Why `/dev/null`.** Without job control, a background job and the
shell would share stdin, and whichever read first would win. POSIX
settles it: an asynchronous list in a non-interactive shell gets
`/dev/null` as stdin, unless you redirect it yourself. With job
control (next milestone), the terminal itself stops a background job
that tries to read, with `SIGTTIN`.

**`nohup` and `disown`.** When an interactive shell exits, it sends
`SIGHUP` to its jobs. `disown %1` removes a job from the table so it
won't get one, and `nohup cmd &` starts it ignoring `SIGHUP` in the
first place. Either way, the job carries on after you log out, with
PID 1 as its new parent.
