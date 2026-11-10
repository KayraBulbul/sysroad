---
title: "Reading clang's output"
minutes: 25
files: [answers.hpp, try.cpp]
deeper:
  - title: Compiler Explorer
    url: https://godbolt.org/
  - title: "Itanium C++ ABI: name mangling"
    url: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
  - title: "What is the PLT? (Eli Bendersky)"
    url: https://eli.thegreenplace.net/2011/11/03/position-independent-code-pic-in-shared-libraries/
---
The listings in the last lesson were cleaned up. Real compiler output
has a lot more in it, and most of it is not instructions. This lesson
is about getting assembly out of your tools and knowing which lines to
skip.

**Three ways to get it.**

- From source: `clang++ -std=c++20 -O1 -S file.cpp -o -` prints
  assembly instead of building a program. (`-o file.s` saves it.)
- From a built program: `objdump -d ./prog` disassembles the machine
  code. It works on both your machines.
- In the browser: Compiler Explorer, godbolt.org, shows source and
  assembly side by side and colours matching lines.

The optimisation level changes everything. At `-O0` every variable
lives in memory and each line of C++ becomes a little load-compute-store
sequence, which is long and tedious to read. `-O1` is the most
readable. `-O2` is what ships.

**What the lines are.** Open `x86-64.s` or `arm64.s` and you'll see
four kinds:

- **Directives** start with a dot: `.globl`, `.p2align`, `.type`,
  `.size`, `.section`, `.cfi_startproc`. They're notes to the
  assembler about alignment, visibility and debugging. Skip them
  all, with one exception below.
- **Labels** end with a colon. A function's name is a label. So are
  jump targets, which clang calls `.LBB0_1` on Linux and `LBB0_1` on
  the Mac, and constants, `.L.str`.
- **Instructions** are the indented lines that aren't directives.
- **Comments** start with `#` on x86-64 and `;` on arm64.

The directives worth reading are the ones that hold **data**:
`.asciz "text"` is a string with its closing zero byte, `.quad 40` is
a 64-bit number and `.long 40` a 32-bit one. That's where string
literals and global variables end up.

**Names are mangled.** C++ lets two functions share a name if their
parameters differ, but the linker needs one unique name each. So the
compiler encodes the namespace and the parameter types into the
symbol: `_Z6squarei` is `square(int)`. `c++filt` decodes them:

```
$ c++filt _Z6squarei
square(int)
```

`extern "C"` turns mangling off for a function, which is why the last
lesson's listings had plain names. macOS adds one more underscore to
the front of every symbol.

**Globals and strings are reached from the instruction pointer.** The
code doesn't know the address it will be loaded at, so it says "the
thing this far from here":

```
leaq  .L.str(%rip), %rax        x86-64: rax = the address of .L.str
movq  counter(%rip), %rax       rax = the value stored at counter
```

```
adrp  x8, _counter@PAGE         arm64: the address, in two halves,
ldr   x9, [x8, _counter@PAGEOFF]   because an instruction is only 4 bytes
```

That's also your first look at **memory operands**. On x86-64,
`movq` with a memory operand loads or stores. On arm64 only two
instructions touch memory: `ldr` loads and `str` stores, and the
address goes in square brackets.

The two listings are the raw output for a file `puzzle.cpp` that you
don't have. Five questions about it are in `answers.hpp`. `try.cpp`
is a scratch file: compile it with `-S`, change it, and watch what the
compiler does.

## Assignment

- [ ] Compile `try.cpp` to assembly at `-O0`, `-O1` and `-O2` and look at the difference
- [ ] Use `c++filt` to name the first function in the listing
- [ ] Find the string, the constant and the global variable in the listing
- [ ] Answer the five questions in `answers.hpp`
