**Precedence.** `&&` and `||` bind equally and left to right, and both
bind tighter than `;` and `&`. So `a || b && c` is `(a || b) && c`,
unlike most programming languages, where `&&` wins. Write it out with
`{ ...; }` or `( ... )` when it matters; conch doesn't have grouping,
but real shells do, and a `( ... )` group runs in a subshell.

**Which commands run in the shell.** POSIX leaves it open whether each
command in a pipeline runs in a subshell. bash runs them all in
children by default, so `echo hi | read x` sets `x` in a child that
then exits. zsh and ksh run the *last* command in the shell itself, so
there it works. `shopt -s lastpipe` gives bash the same behaviour when
job control is off.

**pipefail.** bash's `set -o pipefail` makes a pipeline fail if *any*
command failed, and `${PIPESTATUS[@]}` lists them all. Without it,
`curl ... | tar x` reports success when `curl` fails, as long as `tar`
is happy with what it got. Most "strict mode" scripts start with
`set -euo pipefail` for that reason.

**`set -e` and lists.** With `set -e` a script stops at the first
failing command, except in a condition: anything to the left of `&&`
or `||`, or tested by `if` or `while`. So `false && true` doesn't stop
the script, and its status is still 1. That exception confuses
everyone eventually.
