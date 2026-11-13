---
title: Run a pipeline yourself
kind: project
minutes: 50
files: [pipeline.cpp]
program: pipeline
run:
  args: ["printf", "pear\\napple\\nfig\\napple\\n", "|", "sort", "|", "uniq", "-c", "|", "sort", "-rn"]
deeper:
  - title: "CS:APP 8.4 and 10.9, together"
    url: https://csapp.cs.cmu.edu/
  - title: "Pipelines, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_02
  - title: "The Unix philosophy, from the people who invented pipes (Bell System Technical Journal, 1978)"
    url: https://archive.org/details/bstj57-6-1899
---
`sort | uniq -c | sort -rn` is three programs running at the same
time, each one's stdout connected to the next one's stdin by a pipe.
None of them knows the others exist. Now you know everything a shell
does to set that up, so do it.

`pipeline` takes the commands as words, with a `|` word between them
(quoted, so your shell leaves it alone):

```sh
pipeline printf 'pear\napple\n' '|' sort '|' uniq -c
```

For `n` commands you need `n - 1` pipes. Command `i` reads from pipe
`i - 1` and writes to pipe `i`; the first reads the pipeline's stdin
and the last writes to its stdout. Start them **all** before waiting
for any: each needs the next one reading, or it fills its pipe and
stops.

The hard part is the fds. Every pipe end you create is inherited by
every child you fork after it, and each one is a reason for a `read`
somewhere to never see end of file. Count them as you go: after
forking command `i`, the parent needs nothing of pipe `i - 1` any
more, and only the read end of pipe `i`, for command `i + 1`. A
child needs only its two, already `dup2`'d onto 0 and 1. Close the
rest straight away, and nothing leaks.

A shell reports the **last** command's status as the pipeline's. So
`false | true` succeeds and `true | false` fails. In bash,
`set -o pipefail` makes the pipeline fail if any command did.
`pipeline` prints any command that failed, except one killed by
`SIGPIPE`: that's how `yes | head` ends normally.

This is the first half of the shell you'll build at the end of this
chapter. Write it as if you'll use it there, because you will.

## Assignment

- [ ] `split_pipeline` splits the words at each `|`, and throws for an empty command
- [ ] `run_pipeline` starts every command before it waits for any, connected by pipes
- [ ] Each child resets `SIGPIPE` and exits with 127 if its command isn't found, while the rest carry on
- [ ] No pipe end is left open: not in the parent afterwards, and not in any command
- [ ] If a `pipe` or `fork` fails, the commands already started are killed and reaped before `run_pipeline` throws `std::system_error`
- [ ] Run `pipeline yes '|' head -n 3`, then try forgetting one `close` and run it again
