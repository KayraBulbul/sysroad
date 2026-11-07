---
title: Debugger drills (gdb / lldb)
minutes: 20
xp: 10
files: [answers.hpp, drill.cpp]
deeper:
  - title: "GDB to LLDB command map"
    url: https://lldb.llvm.org/use/map.html
  - title: Beej's Quick Guide to GDB
    url: https://beej.us/guide/bggdb/
---
A debugger lets you stop a running program, look at its variables, and
step through it line by line. On your PC that's **gdb**, and on the Mac
it's **lldb**, which comes with the Command Line Tools. The commands
differ a little, and the ideas are identical.

`drill.cpp` is a small program that crashes. Don't change it. Investigate
it and write four answers in `answers.hpp`. First build it yourself with
debug info and no optimisation, in the lesson folder (`cd "$(sysroad
path)"`):

```text
clang++ -std=c++20 -g -O0 drill.cpp -o drill
gdb ./drill        # PC
lldb ./drill       # Mac
```

**The commands you need**, as gdb / lldb:

- start the program: `run` / `run`
- stop at a function or line: `break main`, `break drill.cpp:25` /
  `b main`, `b drill.cpp:25`
- stop only when a condition holds: `break drill.cpp:25 if i == 37` /
  `b drill.cpp:25 -c 'i == 37'`
- carry on: `continue` (`c`), and step over or into one line: `next` (`n`),
  `step` (`s`)
- print a variable, or *call a function*: `print acc`, `print walk(1, 10)`
  / `p acc`, `expr walk(1, 10)`
- where am I, and how did I get here: `bt` (backtrace) / `bt`
- look at one frame of that backtrace: `frame 1`, `info locals` /
  `frame select 1`, `frame variable`

A function can only be called once the program is running, so `break
main`, `run`, then `print`.

**The questions** are in `answers.hpp`. Each says which command helps.
The tests check a hash of each answer, so a wrong one doesn't reveal the
right one.

## Assignment

- [ ] Build `drill` with `-g -O0` and open it in gdb or lldb
- [ ] Call `walk(1, 1000)` from the debugger
- [ ] Use a conditional breakpoint to read `acc` when `i == 37`
- [ ] Let it crash, then use `bt` and `frame` to find the line and the name
