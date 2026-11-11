---
title: What -O2 and -O3 change
minutes: 35
build: cmake
files: [CMakeLists.txt]
run:
  args: ["16"]
deeper:
  - title: "Clang command line reference: optimisation flags"
    url: https://clang.llvm.org/docs/CommandGuide/clang.html#code-generation-options
  - title: "ThinLTO (clang docs)"
    url: https://clang.llvm.org/docs/ThinLTO.html
  - title: "CheckIPOSupported (cmake.org)"
    url: https://cmake.org/cmake/help/latest/module/CheckIPOSupported.html
---
Everything you've timed so far depended on one switch. What do the
optimisation levels actually turn on?

- **`-O0`** translates each line on its own. Every variable lives in
  memory, so the debugger can show it. Nothing is inlined.
- **`-O1`** keeps values in registers, removes dead code and folds
  constants. The output still follows your code closely.
- **`-O2`** is what releases ship. It adds **inlining** (pasting small
  functions into their callers), hoisting work out of loops, and in
  clang **vectorising** loops to do several elements per instruction.
- **`-O3`** inlines and unrolls more aggressively. Sometimes faster,
  sometimes slower, because bigger code fills the instruction cache.
  Measure before you assume.
- **`-Os`** optimises for size. **`-Og`** is meant for debugging.

Two more flags change the rules rather than the effort:

- **`-march=native`** lets the compiler use every instruction your CPU
  has (AVX2 or AVX-512 on x86, the newest NEON on arm64). The binary
  may then crash with "illegal instruction" on an older machine.
- **`-ffast-math`** lets it treat floating point like real arithmetic:
  reorder sums, assume no NaNs or infinities. Faster, and your results
  change.

Inlining matters most, because it unlocks the rest. Once a function's
body sits inside your loop, the optimiser can see what it does,
vectorise the loop and keep values in registers. But the compiler only
sees **one `.cpp` file at a time**. `brighten()` lives in `pixel.cpp`
and the loop in `main.cpp`, so even at `-O3` every pixel pays for a
real call. **Link-time optimisation** (LTO) fixes that: the compiler
writes its intermediate code into the object files, and the linker
optimises the whole program at once. CMake calls it
`INTERPROCEDURAL_OPTIMIZATION`, and `check_ipo_supported()` tells you
whether this compiler and linker can do it.

CMake's build types bundle the levels: `Debug` is `-g`, `Release` is
`-O3 -DNDEBUG`, and `RelWithDebInfo` is `-O2 -g -DNDEBUG`, optimised
with debug info so profiles have function names. That's the right
default for code you're tuning.

When you're done, build it four ways and compare:

```sh
cmake -S . -B debug -DCMAKE_BUILD_TYPE=Debug && cmake --build debug
cmake -S . -B rel   -DCMAKE_BUILD_TYPE=Release && cmake --build rel
cmake -S . -B lto   -DCMAKE_BUILD_TYPE=Release -DLTO=ON && cmake --build lto
for b in debug rel lto; do ./$b/brighten; done
```

## Assignment

- [ ] With no `CMAKE_BUILD_TYPE` given, the project builds as `RelWithDebInfo`
- [ ] `option(PROFILING ...)`, OFF by default. When ON, everything compiles with `-fno-omit-frame-pointer`
- [ ] `option(LTO ...)`, OFF by default. When ON, and `check_ipo_supported` says yes, turn on `CMAKE_INTERPROCEDURAL_OPTIMIZATION`
- [ ] Build Debug, Release and Release with LTO. How much does each step buy?
