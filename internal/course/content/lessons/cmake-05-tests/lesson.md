---
title: Tests with CTest
base: cmake-04-options
minutes: 25
build: cmake
files: [tests/CMakeLists.txt, tests/test_textutil.cpp, CMakeLists.txt]
run:
  stdin: "one two\nthree\n"
deeper:
  - title: "ctest(1) (cmake.org)"
    url: https://cmake.org/cmake/help/latest/manual/ctest.1.html
  - title: "Test properties (cmake.org)"
    url: https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-tests
---
**CTest** is CMake's test runner. A test is any command, and it passes
when the command exits 0. That's the whole contract, so a test can be a
unit-test program, a script, or your tool run on a sample file:

```cmake
enable_testing()                      # in the top-level CMakeLists.txt

add_executable(geo_tests test_geo.cpp)
target_link_libraries(geo_tests PRIVATE geo)
add_test(NAME geo_tests COMMAND geo_tests)
```

In `add_test`, a target's name stands for the program it builds,
wherever it ends up. Then build, and run every test:

```
cmake --build build
ctest --test-dir build --output-on-failure
```

Exit codes aren't the only way to judge. **Test properties** change
what counts as a pass:

```cmake
add_test(NAME map_paris COMMAND map ${CMAKE_CURRENT_SOURCE_DIR}/data/paris.txt)
set_tests_properties(map_paris PROPERTIES PASS_REGULAR_EXPRESSION "^48\\.85")

add_test(NAME map_no_file COMMAND map missing.txt)
set_tests_properties(map_no_file PROPERTIES WILL_FAIL TRUE)
```

`PASS_REGULAR_EXPRESSION` passes when the output matches. `WILL_FAIL`
flips the result, for a command that *should* fail. And
`${CMAKE_CURRENT_SOURCE_DIR}` is the folder of the `CMakeLists.txt`
being read, so the data file is found wherever the build runs.

Tests get their own folder and `CMakeLists.txt`, which the top one adds
with `add_subdirectory(tests)`. Targets are global, so `tests/` can
link `textutil` and run `wc`.

## Assignment

- [ ] The top-level `CMakeLists.txt` switches testing on and adds the `tests` folder
- [ ] `tests/CMakeLists.txt` builds `textutil_tests` from `test_textutil.cpp`, linking `textutil`, and registers it with `add_test`
- [ ] `test_textutil.cpp` also checks `split_words` and `trim`
- [ ] A test runs `wc` on `data/poem.txt` and checks its output with `PASS_REGULAR_EXPRESSION` (the poem has 5 lines, 14 words, 72 bytes)
- [ ] A test runs `wc` on a file that doesn't exist, and passes because `wc` fails
