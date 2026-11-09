The build type default is the `if` block from the lesson, word for
word. It has to come before any targets.

For the warnings, three lines: `add_library(project_warnings INTERFACE)`,
`target_compile_options(project_warnings INTERFACE ...)`, then add
`project_warnings` to both targets' `target_link_libraries(... PRIVATE ...)`.
---
A sanitizer has to be in both the compile and the link. The compile
flag adds the checks to your code, and the link flag adds the runtime
library that reports them. `add_compile_options` and `add_link_options`
inside `if(SANITIZE)` cover every target made after them, so put the
block above `add_library`.
---
```cmake
option(SANITIZE "Build with AddressSanitizer and UBSan" OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
```
