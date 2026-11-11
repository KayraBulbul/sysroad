---
title: "Symbols, nm and the one-definition rule"
minutes: 30
files: [stats.hpp, stats.cpp, report.cpp]
program: stats
run:
  args: ["3", "1", "4", "1", "5"]
deeper:
  - title: "CS:APP 7.3-7.6: object files, symbols and symbol resolution"
    url: https://csapp.cs.cmu.edu/
  - title: "Definitions and the one-definition rule (cppreference)"
    url: https://en.cppreference.com/w/cpp/language/definition
  - title: "Itanium C++ ABI: name mangling"
    url: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
---
Each object file the compiler makes has a **symbol table**: the names
it *defines* (functions and globals whose code or data is in this
file) and the names it *uses* but expects somewhere else. The linker's
first job is **symbol resolution**: match every use to exactly one
definition. `nm` shows the table:

```sh
clang++ -std=c++20 -c stats.cpp report.cpp main.cpp
nm -C stats.o
```

```text
0000000000000000 T mean(std::vector<double, std::allocator<double> > const&)
0000000000000180 T smallest(std::vector<double, std::allocator<double> > const&)
0000000000000000 D verbosity
                 U __stack_chk_fail
```

`T` is code (text) defined here, `D` initialised data, `B` zeroed data
(bss), `U` undefined: used here, defined elsewhere. Lower-case letters
are **local** symbols that the linker doesn't match across files.
`-C` demangles: in the object file, `smallest` is really
`_Z8smallestRKSt6vectorIdSaIdEE`, which encodes its parameter types.
That **name mangling** is how C++ allows overloads: `f(int)` and
`f(double)` are different symbols.

The rule the linker enforces is C++'s **one-definition rule**: a
function or global variable used across files must be defined in
exactly one of them. This program breaks it four ways. Run it and
read the linker's complaints:

- **A function defined in a header.** `#pragma once` stops a header
  being pasted twice into *one* file, but every `.cpp` that includes
  it gets a copy, so `mean` is defined three times. Mark it `inline`:
  that tells the linker the copies are identical and it may keep one.
- **A variable defined in a header.** Same problem. Either declare it
  `extern int verbosity;` in the header and define it in one `.cpp`,
  or (C++17) make it `inline int verbosity = 1;`.
- **Two helpers with the same name.** `stats.cpp` and `report.cpp`
  each have a private `less_than`, and they do different things. Give
  them **internal linkage**, so each stays a local symbol of its own
  file: put them in an anonymous `namespace { … }`, or mark them
  `static`.
- **A definition that doesn't match its declaration.** `report.cpp`
  defines `report(std::vector<double>&)` but the header promises
  `report(const std::vector<double>&)`. Those mangle differently, so
  `main` uses a symbol nobody defines: an undefined reference. The
  compiler can't catch it, because it only sees one file at a time.

The sanitizers add a marker symbol for every global, so the errors
also mention `__odr_asan_gen_verbosity`. It's the same problem as
`verbosity`, seen twice, and goes away with it.

## Assignment

- [ ] Run it, and match each linker error to one of the four problems
- [ ] `mean` and `verbosity` are each defined once
- [ ] Each `less_than` has internal linkage
- [ ] `report`'s definition matches its declaration
- [ ] Compile with `-c` and compare `nm -C` of each `.o` before and after
