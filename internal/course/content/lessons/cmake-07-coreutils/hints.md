The top-level file sets the project up and hands the rest to the
folders: `add_subdirectory(lib)`, `add_subdirectory(tools)`, then
`enable_testing()` and `add_subdirectory(tests)`. Order matters for one
thing: an INTERFACE target for the warnings has to exist before the
folders that link it are read, so make it first.
---
In `lib/CMakeLists.txt`, paths are relative to `lib/`: the source is
`io.cpp` and the include folder is `include`. The same goes for
`tools/`, where each source is `${name}.cpp` inside the loop.
---
Each tool test is an `add_test` plus a property, like last lessons. For
files, use `${CMAKE_CURRENT_SOURCE_DIR}/data/hello.txt`. A regex can
match a tab: `"1\thello"`, since CMake turns `\t` into a real tab
before ctest sees it.

```cmake
install(TARGETS echo cat wc RUNTIME DESTINATION bin)
```
