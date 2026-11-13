---
title: "Read, parse, run"
kind: capstone
program: conch
minutes: 40
files: [lexer.cpp, exec.cpp]
run:
  interactive: true
deeper:
  - title: "CS:APP 8.4.6: a simple shell (and its shell lab)"
    url: https://csapp.cs.cmu.edu/
  - title: "Write a shell in C (Stephen Brennan)"
    url: https://brennan.io/2015/01/16/write-a-shell-in-c/
  - title: "The POSIX shell spec: what a real one has to do"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html
---
The capstone for this phase is **conch**, a Unix shell. It grows over
eight milestones into something you can actually use: builtins,
redirection, pipelines, background jobs, Ctrl-Z and `fg`, quoting and
variables, history and line editing. Every piece is something you've
already built in this phase. The shell is where they meet.

A shell is a loop: read a line, work out what it says, run it, and go
round again. `main.cpp` is the loop, and it's done for you. It prints
the prompt `conch$ ` when it's talking to a terminal (not when its
input is a file, like the tests'), reads a line, and hands it to your
`run_line`.

This milestone, the "work out what it says" part is simple. Split the
line into **words** at spaces and tabs. A `#` at the *start* of a word
begins a comment that runs to the end of the line (`echo a#b` prints
`a#b`). No quotes yet, no operators: `|` and `>` are just characters in
a word for now. They become special in later milestones.

Then run the words: `fork`, `execvp` in the child, and `waitpid` in the
parent, as in lesson 6.1.4. The command's status becomes `$?`, which
conch keeps in `Shell::last_status`, using the shell's numbering: the
exit code, or 128 plus the signal. If the command can't be found,
print `conch: NAME: command not found` to stderr and exit the child
with 127, as every shell does. A blank line or a comment runs nothing
and leaves `$?` alone. When the input ends, conch exits with `$?`.

`proc.hpp` is the course's version of lesson 6.1.4, so `wait_for`,
`decode` and `shell_status` are there to use.

## Assignment

- [ ] `split_words` splits at spaces and tabs, and stops at a `#` that starts a word
- [ ] `run_command` forks and execs the words, and returns the status as `$?` shows it: 127 with `conch: NAME: command not found` for a missing command
- [ ] `run_line` runs a line, sets `$?`, and runs nothing for a blank line or a comment
- [ ] Run conch, and try `ls -l`, `sleep 1` and a typo
