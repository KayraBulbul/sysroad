---
title: Pipelines
kind: capstone
program: conch
minutes: 50
files: [parser.cpp, pipeline.cpp]
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
a ; b        run a, then b
a && b       run b only if a succeeded ($? is 0)
a || b       run b only if a failed
```

The lexer hands you `|`, `;`, `&&` and `||` as operators too. Parsing
now builds two levels: a **pipeline** is commands joined by `|`, and a
**list** is pipelines joined by the other three. `shell.hpp` has the
shapes: a `List` is a vector of `Step`s, and each `Step` holds a
`Pipeline` and the operator that follows it (`End` for the last). An
operator with nothing before it (`| cat`, `; a`), or with nothing after
it except at the end of the line for `;`, is a `SyntaxError`.

`run_list` walks the steps. Whether a step runs depends on the
operator *before* it and on `$?` right then. A skipped step leaves `$?`
alone, so `false && a && b || c` skips `a` and `b` and still runs
`c`.

`run_pipeline` is lesson 6.1.7's pipeline, with conch's commands. A
single builtin (or a command with no words) runs in the shell, so
`cd` still works. Anything else runs in children, connected by pipes:
fork every command, giving each its pipe ends in the child, close
every end in the parent, then wait for them all. The pipeline's status
is the last command's. If a `pipe` or `fork` fails halfway (out of fds,
say), don't run half a pipeline: kill and reap the commands already
started, print `conch: pipe: <strerror>` and return 1. In the child,
`exec_command` does the rest: it
applies the command's redirections (after the pipes, so `cmd | cat >
out` works) and runs it. That includes builtins, which in a pipeline
run in their child: `exit 4 | cat` ends only that child, as in bash.

## Assignment

- [ ] `parse` splits commands at `|` and pipelines at `;`, `&&` and `||`, and throws `SyntaxError` for an empty command between operators
- [ ] `run_list` runs each pipeline only when the operator before it allows, and stops when `exit` has run
- [ ] `run_pipeline` runs a lone builtin in the shell, and anything else as concurrent children joined by pipes, returning the last one's status
- [ ] No pipe end leaks, `yes | head` finishes, and a pipeline that can't be started returns 1 with a message
