Three commands for the library: `add_library`, then
`target_include_directories` and `target_compile_features`, both
PUBLIC. Two for the program: `add_executable` with just `app/wc.cpp`,
then `target_link_libraries`.
---
If `wc.cpp` can't find `textutil/textutil.hpp`, the include folder is
PRIVATE to `textutil`. If it says `no type named 'span'`, `wc` is being
compiled as C++17, so the C++20 requirement isn't PUBLIC.

A `duplicate symbol` error from the linker means `src/textutil.cpp` is
in both targets. It belongs to the library only.
---
```cmake
add_library(textutil src/textutil.cpp)
target_include_directories(textutil PUBLIC include)
target_compile_features(textutil PUBLIC cxx_std_20)

add_executable(wc app/wc.cpp)
target_link_libraries(wc PRIVATE textutil)
```
