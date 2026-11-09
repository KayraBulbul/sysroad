**How real make finds header dependencies.** You don't list `greet.hpp`
by hand in a real project. The compiler does it: `clang++ -MD -c main.cpp`
writes `main.d`, a little rule listing every header the file included.
The build system reads those files back in. CMake's generated builds do
exactly this, and you'll see `-MD -MF ...d` in the commands.

**Timestamps are a guess.** make trusts mtimes, and they lie: a
`git checkout` sets every file to "now", a clock moves backwards, or a
file is rewritten with the same content. Newer tools like Bazel and
Buck2 hash file contents instead, so an edit that changes nothing
rebuilds nothing.

**Parallel builds.** The dependency graph also says what can run at
once. `main.o` and `greet.o` don't depend on each other, so `make -j8`
or Ninja compiles them side by side. Only the link waits for both.

**Why Ninja?** Ninja is a build system with almost no features: no
variables worth mentioning, no conditions. It's meant to be generated
by something else, like CMake, and in return it starts in milliseconds
even on Chromium's 40,000 files. CMake writes Ninja (or Makefile) rules,
then Ninja does the job you just wrote.
