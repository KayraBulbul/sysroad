---
title: "Build types, warnings and sanitizers"
minutes: 25
build: cmake
files: [CMakeLists.txt]
run:
  stdin: "one two\nthree\n"
deeper:
  - title: "CMAKE_BUILD_TYPE (cmake.org)"
    url: https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html
  - title: "option() (cmake.org)"
    url: https://cmake.org/cmake/help/latest/command/option.html
---
One project, built different ways. A **build type** picks the
optimisation level:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug     # -g: debug info, no optimisation
cmake -S . -B rel   -DCMAKE_BUILD_TYPE=Release   # -O3 -DNDEBUG: fast, asserts off
```

`-D` sets a **cache variable**, a setting stored in the build folder's
`CMakeCache.txt`. Leave the build type out and you get no flags at all:
no debug info and no optimisation, the worst of both. So projects pick
a default when you don't say:

```cmake
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
```

**Your own switches** are `option`s, cache variables that are ON or
OFF. Build with `cmake -S . -B asan -DSANITIZE=ON`, and keep a plain
build next to it in another folder:

```cmake
option(FAST_MATH "Trade float accuracy for speed" OFF)
if(FAST_MATH)
  add_compile_options(-ffast-math)   # every target from here on
endif()
```

**Warnings** belong on your targets, but not on the targets of whoever
uses your library: their code isn't yours to lint. An **INTERFACE
library** builds nothing. It's a bundle of settings that targets take on
by linking it, PRIVATE:

```cmake
add_library(strict INTERFACE)
target_compile_options(strict INTERFACE -Werror)
target_link_libraries(geo PRIVATE strict)
```

sysroad's own builds keep adding the sanitizers for you. The checks
configure your project a few more times, without sysroad's flags, to
see what *your* options do.

## Assignment

- [ ] With no `CMAKE_BUILD_TYPE` given, the project builds as `Debug`
- [ ] An INTERFACE target `project_warnings` with `-Wall -Wextra -Wpedantic`, linked PRIVATE into `textutil` and `wc`
- [ ] `option(SANITIZE ...)`, OFF by default. When ON, everything compiles and links with `-fsanitize=address,undefined`
