---
title: Stepping through assembly in a debugger
minutes: 35
files: [answers.hpp]
program: answers
run: {}
deeper:
  - title: "GDB to LLDB command map"
    url: https://lldb.llvm.org/use/map.html
  - title: "Beej's Quick Guide to GDB"
    url: https://beej.us/guide/bggdb/
  - title: "GDB: examining memory with x"
    url: https://sourceware.org/gdb/current/onlinedocs/gdb.html/Memory.html
---
In lesson 0.1.3 you used the debugger on a program with source and
debug info. It works just as well with neither. You lose line numbers
and variable names, and you keep everything that matters: stop
anywhere, run one instruction at a time, and look at any register or
any byte of memory. Reading a listing tells you what the code *would*
do. The debugger shows you what it *did*, with the real values.

This lesson's program is a **vault**. It takes three arguments, a
word, a number and an index, and checks them in three phases. You
have no source, only its assembly. Build the one for your machine, in
the lesson folder (`cd "$(sysroad path)"`):

```text
clang++ vault-x86-64.s -o vault      # PC
clang++ vault-arm64.s -o vault       # Mac
./vault hello 1 2
```

Then start it under the debugger with some guesses as arguments:

```text
gdb --args ./vault hello 1 2         # PC
lldb -- ./vault hello 1 2            # Mac
```

**The commands**, as gdb / lldb:

- stop at a function: `break phase_1` / `b phase_1`, then `run`
- show the instructions of the function you're in, with an arrow at
  the next one: `disassemble` / `disassemble` (or `di`)
- run one instruction: `stepi` / `si`. To run one instruction but
  treat a whole call as one step: `nexti` / `ni`
- stop at an exact instruction: `break *0x5555555551ec`, using an
  address from the disassembly / `b -a 0x100003f20`
- print a register: `p $rax`, `p/x $rax` for hex / `p $x0`, `p/x $x0`
- print all of them: `info registers` / `register read`
- look at memory: `x/s $rsi` shows a string at the address in `rsi`.
  `x/8dg $rsp` shows 8 numbers ("d" for decimal, "g" for 8 bytes
  each). `x/16xb $rsp` shows 16 bytes in hex. lldb accepts the same
  `x/…` commands
- run until the current function returns: `finish` / `finish`
- carry on: `continue` / `c`. Start again: `run`

gdb prints `mov` and `push` where clang printed `movq` and `pushq`.
It leaves the size letter out when the registers already make it
obvious.

**The idea for each phase** is the same: let the program do the work,
stop just before it compares your guess with the right answer, and
read the right answer out of a register or memory.

- **Phase 1** builds the secret word in memory and calls `strcmp`.
  Both strings are arguments to that call.
- **Phase 2** calls a function, `forge`, and compares what it returns
  with your number.
- **Phase 3** follows a table of numbers, starting at your index, and
  wants to end up at a particular value. This one you have to think
  about.

Put your three answers in `answers.hpp`. **Run** prints the command
that tries them on your vault.

## Assignment

- [ ] Build `vault` from the assembly for your machine
- [ ] Phase 1: stop at `strcmp` and read the word it compares yours with
- [ ] Phase 2: stop after the call to `forge` and read its result
- [ ] Phase 3: dump `table`, read `phase_3`, and find the index that works
- [ ] All three answers in `answers.hpp`, and `./vault` says the vault is open
