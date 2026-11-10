---
title: "CS:APP Bomb Lab"
kind: lab
minutes: 360
deeper:
  - title: "Bomb Lab writeup (CMU)"
    url: http://csapp.cs.cmu.edu/3e/bomblab.pdf
  - title: "CS:APP lab page, with every self-study handout"
    url: http://csapp.cs.cmu.edu/3e/labs.html
  - title: "GDB to LLDB command map"
    url: https://lldb.llvm.org/use/map.html
---
The vault in 4.2.7 was a small version of this lab. The **bomb** is a
Linux x86-64 program with six phases and no source. Each phase reads
one line and checks it. A right line defuses that phase, and a wrong
one makes the bomb "explode": it prints `BOOM!!!` and quits. Your job
is to work out the six lines by reading the machine code.

You'll use all of chapter 4.2. Calls and arguments (4.2.4), loops
and branches (4.2.3), arrays, structs and pointers (4.2.5),
optimised idioms like jump tables (4.2.6), and the debugger (4.2.7).

**Get it.** The self-study bomb is the same for everyone and runs on
the PC:

```text
mkdir -p ~/csapp && cd ~/csapp
curl -LO http://csapp.cs.cmu.edu/3e/bomb.tar
tar xf bomb.tar && cd bomb
```

`bomb.c` is only `main`, to show you the shape. The phases are in the
binary.

**Before you run anything**, set a breakpoint on the function that
explodes, so a wrong guess stops in the debugger instead of
finishing:

```text
gdb ./bomb
(gdb) break explode_bomb
(gdb) break phase_1
(gdb) run
```

The self-study bomb doesn't report to a server, so an explosion costs
you nothing except a restart. Still, the breakpoint is the habit the
lab is built to teach.

**Useful moves.**

- `objdump -d bomb > bomb.s` gives you the whole listing to read in
  your editor. `objdump -t bomb` lists the functions by name, which is
  a map of the program.
- Read a phase's listing before you step through it. Find the calls,
  the compares that lead to `explode_bomb`, and the loop back-edges,
  the same way you did in 4.2.3.
- Constants that look like addresses usually are. `x/s 0x402400`
  prints a string, and `x/8wd` prints a few `int`s.
- Some phases call helpers (`read_six_numbers`, `string_length`,
  `func4`). Work out what each helper does once, write it down as
  C++, and reuse that.
- Put your answers in a file, one per line, and run `./bomb
  answers.txt`. It reads lines from the file first, then from the
  keyboard, so you don't retype solved phases.
- No looking up answers. Plenty of solved bombs are online, and reading
  them skips the whole point.

**Done when** all six phases are defused. There's also a hidden
"secret phase"; finding it is optional and good practice. Tick this
step off by hand.
