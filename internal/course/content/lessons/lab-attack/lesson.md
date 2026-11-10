---
title: "CS:APP Attack Lab"
kind: lab
minutes: 360
deeper:
  - title: "Attack Lab writeup (CMU), read this first"
    url: http://csapp.cs.cmu.edu/3e/attacklab.pdf
  - title: "CS:APP lab page, with every self-study handout"
    url: http://csapp.cs.cmu.edu/3e/labs.html
  - title: "Security Technologies: Stack Smashing Protection (Red Hat)"
    url: https://www.redhat.com/en/blog/security-technologies-stack-smashing-protection-stackguard
---
This is the last of CMU's three machine-code labs, and it's about the
stack. You get two small Linux x86-64 programs, `ctarget` and
`rtarget`. Each reads a line into a fixed-size buffer on the stack
with a function that doesn't check the length. You'll write inputs
that overrun the buffer and change what the program does next, for
example making a function return somewhere it wasn't meant to.

The point isn't the trick. It's seeing exactly what a stack frame
holds. In 2.2.1 and 4.2.4 you read the frame: the return address
`call` pushes, the saved registers, and the locals below them. Here you find out
what happens when a write goes past the end of a local. That's also
why the bugs the sanitizers caught in phase 0 matter so much in real
programs.

**The lab's own sandbox.** The targets were built for teaching. They
turn off the protections a normal build has, such as stack canaries,
and they use fixed addresses. So the exercises work on these binaries
and only on them. The writeup explains each one and which protection
the later phases have to work around.

**Get it.** It runs on the PC:

```text
mkdir -p ~/csapp && cd ~/csapp
curl -LO http://csapp.cs.cmu.edu/3e/target1.tar
tar xf target1.tar && cd target1
```

Always run the targets with **`-q`**. The handout was made for CMU's
autograder, and without `-q` the programs try to report to its server
and fail:

```text
./hex2raw < answer.txt | ./ctarget -q
```

You write each input as hex bytes in a text file, and `hex2raw` turns
them into raw bytes. That's why `answer.txt` holds text like
`30 31 32 …`, not characters.

**The five phases**, per the writeup. Phases 1–3 use `ctarget` and
put your own code on the stack. Phases 4–5 use `rtarget`, which
doesn't allow that, so you chain together short pieces of code
already in the program. These are listed in `farm.c`. Work through
them in order. Each one adds a single idea to the last.

**Useful moves.**

- Read the writeup end to end before you start. It gives the target
  functions, the rules for each phase, and how to turn assembly into
  bytes (`gcc -c` then `objdump -d`).
- Use gdb to see the frame right after the read: `break getbuf`,
  `run -q`, `disassemble`, then `break *` the instruction after the
  call to `Gets` and `continue`. Now `x/8gx $rsp` shows the buffer
  and what sits above it.
- Remember x86-64 is little-endian (2.1.4):
  addresses go into the input least-significant byte first.
- As with the other labs, solve it yourself. Reading someone else's
  answer teaches you nothing about stack frames.

**Done when** all five phases print `PASS`. Tick this step off by
hand.
