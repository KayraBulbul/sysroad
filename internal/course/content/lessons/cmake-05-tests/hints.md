Two lines go at the end of the top-level `CMakeLists.txt`:
`enable_testing()` then `add_subdirectory(tests)`. `enable_testing()`
has to be in the top folder, because `ctest` looks for the test list
there.
---
In `tests/CMakeLists.txt`, the unit tests are three lines:
`add_executable`, `target_link_libraries`, `add_test(NAME ... COMMAND ...)`.
`check.hpp` is in the same folder as the test file, so it's found
without any include path.
---
`wc` prints the counts right-aligned in 7 columns, so the line starts
with spaces: `^ +5 +14 +72 `. In a CMake string, a backslash needs
doubling, but this pattern doesn't use any.

```cmake
add_test(NAME wc_missing_file COMMAND wc no-such-file.txt)
set_tests_properties(wc_missing_file PROPERTIES WILL_FAIL TRUE)
```
