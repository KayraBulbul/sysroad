---
title: A first CMakeLists.txt
minutes: 20
build: cmake
files: [CMakeLists.txt]
run:
  args: [Ada, "world?"]
deeper:
  - title: "CMake tutorial, step 1 (cmake.org)"
    url: https://cmake.org/cmake/help/latest/guide/tutorial/A%20Basic%20Starting%20Point.html
  - title: "An Introduction to Modern CMake (Henry Schreiner)"
    url: https://cliutils.gitlab.io/modern-cmake/
---
CMake doesn't build anything itself. It's a **build system
generator**: you describe your project in `CMakeLists.txt`, and CMake
writes the rules for a real build tool, Ninja or make, like the ones
your `minimake` read. That's two steps:

```
cmake -S . -B build        # configure: read CMakeLists.txt, write build/
cmake --build build        # build: run Ninja or make in build/
./build/hello Ada
```

Everything CMake makes goes in `build/`, so deleting that folder always
gets you a clean start. (sysroad's data repo ignores `build/`, so you can
try these commands in your lesson folder: `cd "$(sysroad path)"`.)

A whole project can be four commands:

```cmake
cmake_minimum_required(VERSION 3.20)
project(tool LANGUAGES CXX)

add_executable(tool main.cpp parse.cpp)
target_compile_features(tool PRIVATE cxx_std_20)
```

- `cmake_minimum_required` comes first, always. It also switches on the
  behaviour of that CMake version, so new releases don't change what
  your file means.
- `project` names the project and its languages.
- `add_executable` makes a **target**: something to build, named
  `tool`, from those sources. You list `.cpp` files only; headers are
  found through `#include`.
- `target_compile_features` says what the target needs. CMake picks the
  flag (`-std=gnu++20` here) for whichever compiler it finds.

From here on, sysroad grades CMake lessons by running those two
commands on your project. It adds `-g -O0` and the sanitizers itself,
through `CMAKE_CXX_FLAGS`, so you don't have to.

## Assignment

- [ ] `CMakeLists.txt` starts with `cmake_minimum_required(VERSION 3.20)` and names the project
- [ ] It builds a program target named `hello` from `main.cpp` and `greet.cpp`
- [ ] `hello` is built as C++20 (`greet.cpp` uses `starts_with`, which C++17 doesn't have)
