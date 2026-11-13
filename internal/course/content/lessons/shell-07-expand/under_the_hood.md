**Word splitting.** In sh and bash, an *unquoted* expansion is split
into several words at spaces, tabs and newlines (`$IFS`), and each
word is then matched as a file pattern. So `rm $file` with
`file='my notes.txt'` removes `my` and `notes.txt`. That's why every
style guide says "quote your variables", and why zsh doesn't split
by default. conch follows zsh.

**The order of expansions.** POSIX does tilde, then parameters,
command substitution `$(...)` and arithmetic `$((...))`, then field
splitting, then pathname expansion (`*.txt`), then quote removal. Each
happens on the result of the one before, which is how `"$(ls)"` keeps
the newlines and `$(ls)` doesn't.

**Command substitution.** `$(cmd)` runs `cmd` with stdout on a pipe
and replaces itself with the output, minus trailing newlines: your
`capture` from lesson 6.1.6. It runs in a subshell, a forked copy of
the shell, so `$(cd /tmp)` changes nothing.

**Why the lexer can't expand.** A line is read once, but its
commands run one at a time, and each can change what later words
mean: `X=1; echo $X`, or `false; echo $?`. So a shell parses the
whole line first and expands each command just before running it.
`set -x` shows the expanded command, which is the fastest way to see
what the shell really ran.
