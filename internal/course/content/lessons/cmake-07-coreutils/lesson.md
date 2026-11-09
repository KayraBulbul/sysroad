---
title: Your coreutils as one CMake project
kind: project
minutes: 45
build: cmake
files: [CMakeLists.txt, lib/CMakeLists.txt, tools/CMakeLists.txt, tests/CMakeLists.txt]
run:
  args: ["-n", "poem.txt"]
  files:
    poem.txt: "The fog comes\non little cat feet.\n"
deeper:
  - title: "install() (cmake.org)"
    url: https://cmake.org/cmake/help/latest/command/install.html
  - title: "foreach() (cmake.org)"
    url: https://cmake.org/cmake/help/latest/command/foreach.html
---
Phase 1's tools, as one project. `echo`, `cat` and `wc` are written,
and so is a small library they share, `coreutils`, with the "open each
file or read standard input" loop they'd otherwise each repeat. What's
missing is the build. You write all of it, from an empty
`CMakeLists.txt` in each folder:

```
CMakeLists.txt          the project, settings, add_subdirectory for each folder
lib/CMakeLists.txt      the coreutils library (include/ holds its header)
tools/CMakeLists.txt    echo, cat and wc
tests/CMakeLists.txt    CTest tests that run the tools
```

Three programs built the same way is a loop. CMake has one:

```cmake
foreach(name alpha beta)
  add_executable(${name} ${name}.cpp)
endforeach()
```

The last new piece is **install**: copying the finished programs to
where programs live. `install(TARGETS ...)` records the rule, and
`cmake --install build --prefix ~/.local` runs it, putting the
programs in `~/.local/bin`:

```cmake
install(TARGETS alpha beta RUNTIME DESTINATION bin)
```

## Assignment

- [ ] A library target `coreutils` from `lib/io.cpp`, sharing its `include` folder and C++20 with whoever links it
- [ ] Program targets `echo`, `cat` and `wc` from `tools/`, each linking `coreutils`
- [ ] Everything you build has `-Wall -Wextra`, the way last lessons did it
- [ ] CTest tests in `tests/` run the tools: at least three, all passing (`tests/data/hello.txt` is there to use)
- [ ] `cmake --install` puts `echo`, `cat` and `wc` in the prefix's `bin/`
