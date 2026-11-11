The default build type is lesson 3.1.4's `if` block with
`RelWithDebInfo` in place of `Debug`. It goes before any targets.

`PROFILING` follows the same pattern as `SANITIZE` in that lesson, with
only a compile option: frame pointers don't need anything at link time.
---
`CMAKE_INTERPROCEDURAL_OPTIMIZATION` is the default for the
`INTERPROCEDURAL_OPTIMIZATION` property of every target created after
it's set, so set it above `add_library`:

```cmake
option(LTO "Optimise across source files at link time" OFF)
if(LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT lto_ok OUTPUT lto_error)
  if(lto_ok)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  else()
    message(WARNING "LTO isn't supported here: ${lto_error}")
  endif()
endif()
```
