The `FetchContent` block goes in the top-level `CMakeLists.txt`, before
`add_subdirectory(tests)`, since the tests link doctest's targets. The
name in `FetchContent_Declare(doctest ...)` matters: it's how the
download is cached and found.
---
In the test file, replace `#include "check.hpp"` with
`#include <doctest/doctest.h>`, delete `main`, and put the checks into
`TEST_CASE("...") { ... }` blocks. doctest's `CHECK` has the same name
as yours, and its `REQUIRE` stops the test case on failure, for
checks the rest depends on (like a size before indexing).
---
```cmake
include(FetchContent)
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(doctest)
```
