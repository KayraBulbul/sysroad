**`$?` after Ctrl-Z.** bash sets `$?` to 148 after a Ctrl-Z on Linux
(128 + 20, `SIGTSTP`) and 146 on the Mac, where `SIGTSTP` is 18. Signal
numbers differ between systems, which is why scripts compare against
`$(kill -l TSTP)` rather than a number.

**Job specs.** bash accepts `%1`, `%%` or `%+` (the current job), `%-`
(the previous one), `%sleep` (a job whose command starts with
`sleep`) and `%?pattern` (one containing it). The "current job" is
the one most recently stopped or put in the background, which `jobs`
marks with `+`.

**Why a stopped job's group gets SIGHUP then SIGCONT.** A stopped
process can't act on `SIGHUP` until it runs again, so the shell
continues it straight after, and the default action ends it. The
kernel does the same for orphaned process groups, which is what a
stopped job becomes when its shell exits without asking.

**Shells without job control.** Scripts run with job control off:
every command stays in the shell's own process group, so a Ctrl-C at
the terminal reaches the script *and* the command it's running, and
both stop. That's what you want: Ctrl-C ends the whole script.
`set -m` turns job control on in a script, if you really need `fg`.
