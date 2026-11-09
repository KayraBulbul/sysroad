---
title: Why build systems exist
minutes: 25
files: [minimake.cpp]
run:
  stdin: "time main.cpp 10\ntime greet.cpp 10\ntime greet.hpp 30\ntime main.o 20\ntime greet.o 20\ntime app 25\n\napp: main.o greet.o\n  c++ main.o greet.o -o app\nmain.o: main.cpp greet.hpp\n  c++ -c main.cpp\ngreet.o: greet.cpp greet.hpp\n  c++ -c greet.cpp\n"
deeper:
  - title: "Recursive Make Considered Harmful (Peter Miller)"
    url: https://aegis.sourceforge.net/auug97.pdf
  - title: "Build Systems à la Carte (Mokhov, Mitchell, Peyton Jones)"
    url: https://www.microsoft.com/en-us/research/uploads/prod/2018/03/build-systems.pdf
---
So far sysroad has built your code with one command:
`clang++ *.cpp -o prog`. That compiles every file, every time. Fine for
three files, slow for three thousand. Chromium takes hours to build from
scratch, so nobody can afford to do that after changing one line.

The fix is **separate compilation**. Each `.cpp` compiles on its own
into an object file (`.o`), and a final **link** step joins the objects
into the program:

```
c++ -c main.cpp      ->  main.o
c++ -c greet.cpp     ->  greet.o
c++ main.o greet.o -o app
```

Now an edit to `greet.cpp` only needs `greet.o` rebuilt, then the link.
But someone has to know that. And headers make it harder: `#include`
pastes `greet.hpp` into both `.cpp` files, so changing the header means
recompiling both.

That bookkeeping is what a build system does. `make`, the 1976
original, works from **rules**: a target, the files it depends on, and
the command that makes it. It rebuilds a target when the target is
missing, or when any dependency is newer than it, after bringing the
dependencies themselves up to date first.

You'll write that core: `plan` takes the rules, the time each file last
changed, and a goal, and returns the commands to run, in order.

```
$ ./minimake < Buildfile        # greet.hpp changed at time 30
c++ -c main.cpp
c++ -c greet.cpp
c++ main.o greet.o -o app
```

The rest of this chapter is CMake, which writes these rules for you.

## Assignment

- [ ] Bring each dependency up to date first, in the order the rule lists them
- [ ] Run a rule's command when its target is missing, or a dependency is newer than it (the same time doesn't count), or a dependency was just rebuilt
- [ ] Run each command at most once, even if many targets need it
- [ ] Throw `std::runtime_error` with `no rule to make NAME` for a missing file no rule makes, and `cycle: a -> b -> a` when the rules loop
