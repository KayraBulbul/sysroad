---
title: "Read, parse, run"
kind: capstone
project: conch
program: conch
minutes: 50
contract:
  header: conch.hpp
  types: [Shell]
  functions:
    - name: run_line
      sig: "int(Shell&, const std::string&)"
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
The capstone for this phase is **conch**, a Unix shell. It grows into
something you can actually use: builtins, redirection, pipelines,
background jobs, Ctrl-Z and `fg`, quoting and variables, history and
line editing. Every piece is something you've already built in this
phase. The shell is where they meet. Like kite, conch is a project: its
lessons share one folder, which starts empty, and every line is yours.

A shell is a loop: read a line, work out what it says, run it, and go
round again. When the input ends, the shell exits with `$?`, the last
command's status.

**Reading.** conch reads its input, fd 0, one byte at a time with
`read`, up to each `\n`. That's slow, but it's the only way to be
right: the commands conch runs share that input, and `head -n 1` on
the next line expects to get the line after it, not whatever conch read
ahead into a buffer. A last line without a `\n` still counts. When its
input is a terminal (`isatty(0)`), conch prints the prompt `conch$ `
before each line, and flushes it, since nothing ends the line. When
the input is a file or a pipe, as in the tests' scripts, it prints no
prompt.

**Splitting.** This lesson, working out what a line says is simple.
Split it into **words** at spaces and tabs. A `#` at the *start* of a
word begins a comment that runs to the end of the line (`echo a#b`
prints `a#b`). Quotes, `|`, `>` and the rest come in later lessons.

**Running.** Then run the words: `fork`, `execvp` in the child, and
`waitpid` in the parent, as in lesson 6.1.4 (bring your code over, or
write it again). The command's status becomes `$?`, numbered the way
shells do: the exit code, or 128 plus the signal that killed it. If the
command can't be found (`execvp` fails with `ENOENT`), the child prints
`conch: NAME: command not found` to stderr and exits with 127; if it
can't be run for another reason, it prints `conch: NAME:` and the
reason (`strerror`) and exits with 126. A blank line or a comment runs
nothing and leaves `$?` alone.

What conch remembers between lines lives in a struct, `Shell`: for now
just `$?`, more later. Its design is yours, and so is how you split the
code into files.

## Contract

```cpp
// conch.hpp
struct Shell { /* yours */ };

// Runs one line of input in the shell, and returns $? after it.
int run_line(Shell& sh, const std::string& line);
```

And in a `.cpp` file, `int main()`, with no parameters, ending in
`return`: the tests call your `main` renamed, and only the real `main`
returns 0 when it runs off its end. The tests run your whole program
on scripts and on a terminal of their own, and call `run_line` on a
`Shell sh;` of their own to check what only the shell process itself
can show. So `Shell` needs to work without arguments.

## Assignment

- [ ] conch reads its input a byte at a time and runs each line, prints `conch$ ` first when the input is a terminal, and at the end of the input exits with `$?`
- [ ] A line is split into words at spaces and tabs, and a `#` at the start of a word ends it
- [ ] A command runs in a child; its status is the exit code, or 128 plus the signal
- [ ] A command that isn't found prints `conch: NAME: command not found` to stderr and has status 127; one that can't be run says why and has 126
- [ ] A blank line or a comment runs nothing, and `$?` stays as it was
