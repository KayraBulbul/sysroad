---
title: Check your toolchain
kind: setup
minutes: 5
check: toolchain
---
Every lesson builds your code with **clang++** using the same flags:

```text
-std=c++20                       the language version
-g -O0                           debug info, no optimisation
-fsanitize=address,undefined     stop on memory bugs and undefined behaviour
```

The sanitizers make bugs that C would silently "get away with" (reading
past an array, using freed memory, overflowing a signed int) crash
immediately with a report that points at the line.

Submit here doesn't grade code. It checks that this machine can build and
run a C++20 program with sanitizers, that the test framework compiles, and
that a debugger is installed. Do it once on each machine.

> Missing something? Omarchy: `sudo pacman -S clang gdb`. Mac: `xcode-select --install`.

## Assignment

- [ ] Submit (`s` in the sidebar, or `sysroad submit` in a shell)
- [ ] Fix anything marked ✗ and submit again
