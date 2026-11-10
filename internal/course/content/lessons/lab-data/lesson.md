---
title: "CS:APP Data Lab"
kind: lab
minutes: 300
deeper:
  - title: "Data Lab writeup (CMU)"
    url: http://csapp.cs.cmu.edu/3e/datalab.pdf
  - title: "CS:APP lab page, with every self-study handout"
    url: http://csapp.cs.cmu.edu/3e/labs.html
  - title: "Bit Twiddling Hacks (Sean Anderson)"
    url: https://graphics.stanford.edu/~seander/bithacks.html
---
The Data Lab comes from CMU's systems course (15-213, the course
behind the *CS:APP* textbook). It's thirteen puzzles in one C file,
`bits.c`. Each puzzle is a function you fill in. The catch is the
rules:

- **Integer puzzles:** no `if`, no loops, no `-`, no casts, no
  constants above `0xFF`. Only `! ~ & ^ | + << >>`, and a limit on how
  many you use (`isLessOrEqual` gets 24).
- **Float puzzles:** you get `if` and loops, but no `float` type. A
  float arrives as its 32 raw bits in an `unsigned`, and you work on
  those bits directly.

Chapter 4.1 gave you everything you need. Masks and shifts (4.1.2),
two's complement and `TMin` (4.1.3), and sign, exponent and fraction
fields (4.1.5). The lab makes you use them with almost no tools.

**Get it.** The self-study handout runs on the PC, not the Mac: the
checker `dlc` is a Linux x86-64 program.

```text
mkdir -p ~/csapp && cd ~/csapp
curl -LO http://csapp.cs.cmu.edu/3e/datalab-handout.tar
tar xf datalab-handout.tar && cd datalab-handout
make
```

`make` builds 32-bit programs (`-m32`). If it fails with a missing
`bits/libc-header-start.h` or `-lgcc`, either install the 32-bit
libraries (`sudo pacman -S lib32-glibc lib32-gcc-libs` with the
`multilib` repo on) or delete `-m32` from the `Makefile`. `int` is 32
bits either way, so the puzzles behave the same.

**The loop.** Read the rules at the top of `bits.c` first. Then, for
each puzzle:

```text
./dlc bits.c               # did I break a rule?
make && ./btest -f bitXor  # is this one function right?
./driver.pl                # everything: correctness + operator counts
```

`./btest -f name -1 5 -2 7` runs one function on the arguments you
give, which beats rereading code when a test fails. `./ishow 0x80000000`
and `./fshow 0x7f800000` print a number's bits as an integer and as a
float.

**Advice.**

- Do them in file order. Later puzzles reuse ideas from earlier ones.
- Write the plain-C version first, with `if`s and `-`, to pin down
  what the answer is. Then rewrite it under the rules.
- `!!x` turns any value into 0 or 1. `x >> 31` (on an `int`) gives
  all zeros or all ones. Those two tricks make up half the lab.
- For the float puzzles, split `uf` into its three fields with masks
  first, deal with the special cases (0, denormals, infinity and NaN),
  then the normal case.
- Don't look up solutions. Every puzzle is solved online, and none of
  it sticks if you read the answer.

**Done when** `./driver.pl` prints full marks for correctness, 36/36.
The performance column is for operator counts. Getting under the
limits is the target. Getting well under them is a bonus. Tick this
step off by hand when you're there.
