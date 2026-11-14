---
title: Dependencies with FetchContent
base: cmake-05-tests
minutes: 25
build: cmake
files: [CMakeLists.txt, tests/CMakeLists.txt, tests/test_textutil.cpp]
solution_removes: [tests/check.hpp]
run:
  stdin: "one two\nthree\n"
deeper:
  - title: "FetchContent (cmake.org)"
    url: https://cmake.org/cmake/help/latest/module/FetchContent.html
  - title: "doctest tutorial"
    url: https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
---
C++ has no `go get` or `pip install`. The language doesn't know about
packages, so every build system grew its own answer. CMake's built-in
one is **FetchContent**: download a project at configure time and add
it to your build, as if it were one of your folders.

```cmake
include(FetchContent)
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.1.4
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fmt)

target_link_libraries(report PRIVATE fmt::fmt)
```

`FetchContent_Declare` says where the code is. Pin a release tag (or a
commit), never a branch: a build should get the same code every time.
`FetchContent_MakeAvailable` downloads it into `build/_deps/` and adds
its targets, which you link like your own. `fmt::fmt` has a `::`
because it's an **alias**, a read-only name a project exports. A typo in
an alias is an error at configure time, whereas a typo in a plain name
would turn into a confusing `-lfmtt` at link time.

Your `check.hpp` was fine for six checks. A real test framework names
each test, shows the values when a check fails, and keeps going after a
failure. sysroad has graded you with **doctest** all along; now your own
tests use it too.

```cpp
#include <doctest/doctest.h>

TEST_CASE("area of a square") {
    CHECK(area(Square{3}) == 9);
}
```

doctest makes a `doctest::doctest_with_main` target that brings a
`main()` which runs every `TEST_CASE`. Link that instead of writing
`main` yourself.

## Assignment

- [ ] Fetch doctest `v2.4.11` from `https://github.com/doctest/doctest.git` with FetchContent, under the name `doctest`
- [ ] `textutil_tests` links `doctest::doctest_with_main`
- [ ] `test_textutil.cpp` uses doctest: a `TEST_CASE` for each of `count`, `split_words` and `trim`, and no `check.hpp`
- [ ] All three CTest tests still pass
