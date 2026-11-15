---
title: Pipelines
kind: capstone
project: conch
program: conch
minutes: 60
run:
  interactive: true
deeper:
  - title: "Pipelines and lists, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_02
  - title: "bash's exec loop: execute_cmd.c (source)"
    url: https://git.savannah.gnu.org/cgit/bash.git/tree/execute_cmd.c
---
Now conch learns `|`, and the three ways to join commands on one line:

```text
a | b        a's stdout is b's stdin, and both run at once
a ; b        run a, then b
a && b       run b only if a succeeded ($? is 0)
a || b       run b only if a failed
```

**Tokens.** Your lexer learns four more operators: `|`, `||`, `&&` and
`;`. Like the redirections, they end a word and need no spaces around
them. (A single `&` comes next lesson.)

**Parsing** builds two levels now: a **pipeline** is commands joined by
`|`, and a **list** is pipelines joined by `;`, `&&` and `||`. One
shape that works: a list is a vector of steps, and each step holds a
pipeline and the operator that follows it, with "end of line" for the
last. A command can't be empty: an operator with nothing before it
(`| cat`, `; a`, `a | | b`) is a syntax error, and so is `|`, `&&` or
`||` at the end of the line. A `;` at the end is fine.

**Running a list.** Whether a step runs depends on the operator
*before* it and on `$?` right then. A skipped step leaves `$?` alone,
so `false && a && b || c` skips `a` and `b` and still runs `c`. After
`exit`, nothing more on the line runs.

**Running a pipeline** is lesson 6.1.7's pipeline, with conch's
commands. A single builtin still runs in the shell, so `cd` still
works. Anything else runs in children, connected by pipes: fork every
command, giving each its pipe ends in the child, close every end in the
parent as soon as the child has it, then wait for them all. The
pipeline's status is the last command's. In the child, apply the
command's redirections *after* the pipe's `dup2`s, so `cmd | cat > out`
works. A builtin in a pipeline runs in its child, like any other
command there: `exit 4 | cat` ends only that child, and `cd / | true`
moves nobody, as in bash.

If a `pipe` or `fork` fails halfway (out of fds, say), don't run half a
pipeline: kill and reap the commands already started, print
`conch: pipe:` (or `fork:`) and the reason, and give the pipeline
status 1.

## Contract

Nothing new. Some tests run pipelines in a `Shell` of their own, timing
them and checking the test process's fds afterwards.

## Assignment

- [ ] `|`, `;`, `&&` and `||` split a line, with or without spaces, and an empty command between them is a syntax error
- [ ] Each pipeline in a list runs only when the operator before it allows; a skipped one leaves `$?` alone, and `exit` ends the list
- [ ] A lone builtin runs in the shell; any other pipeline runs as children joined by pipes, all at once, with the last one's status
- [ ] The shell keeps no pipe ends open, `yes | head` finishes, and a pipeline that can't be started runs nothing, says why and has status 1
