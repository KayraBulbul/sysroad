**Why a regex and not the exact output?** CTest has no "expected
output" property, and matching the whole output breaks on every
harmless change. Projects that need exact output write a small script
that runs the tool and diffs, and `add_test` runs the script. CMake can
be that script: `cmake -P compare.cmake`.

**`include(CTest)` vs `enable_testing()`.** `include(CTest)` calls
`enable_testing()` for you, adds a `BUILD_TESTING` option (ON by
default) so others can switch your tests off, and sets up dashboard
reporting. Wrap the `add_subdirectory(tests)` in `if(BUILD_TESTING)`,
and a project that pulls yours in as a dependency won't build your
tests.

**Running some tests.** `ctest -R wc` runs tests whose names match a
regex, `-E` excludes, `-j8` runs eight at once, and `--rerun-failed`
runs only what failed last time. `LABELS` group tests (`unit`,
`slow`) for `ctest -L unit`.

**What sysroad checks.** It asks `ctest --show-only=json-v1` for the
list of tests and their properties, then runs `ctest`. The same JSON is
how IDEs fill their test panels.
