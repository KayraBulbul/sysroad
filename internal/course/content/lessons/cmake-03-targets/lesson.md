---
title: "Targets, libraries and include paths"
minutes: 25
build: cmake
files: [CMakeLists.txt]
run:
  stdin: "one two\nthree\n"
deeper:
  - title: "target_link_libraries and usage requirements (cmake.org)"
    url: https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#transitive-usage-requirements
  - title: "Effective Modern CMake (Manuel Binna)"
    url: https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1
---
This project is a library of text functions, `textutil`, and a `wc`
built on it. The starter `CMakeLists.txt` compiles everything into one
program, with `include_directories` handing every header to everyone.
It works, but the tests can't reuse the text code without compiling it
again, and neither could a second tool.

Split it into two **targets**:

```cmake
add_library(geo src/geo.cpp)
target_include_directories(geo PUBLIC include)
target_compile_features(geo PUBLIC cxx_std_20)

add_executable(map app/map.cpp)
target_link_libraries(map PRIVATE geo)
```

`add_library` builds `src/geo.cpp` once, into `libgeo.a`. Then
`target_link_libraries(map PRIVATE geo)` does more than link it: `map`
also gets everything `geo` marked **PUBLIC**. The keyword says who a
setting is for:

- **PRIVATE**: only for building this target
- **PUBLIC**: for this target, and for every target that links it
- **INTERFACE**: only for the targets that link it (a header-only library)

`geo`'s header is in `include/`, and anyone who links `geo` includes
it, so the include folder is PUBLIC. The header uses `std::span`, so
everyone who includes it needs C++20: PUBLIC too. A detail only the
`.cpp` needs, like an extra warning flag, would be PRIVATE.

The lesson's tests are a third target that links `textutil`. They only
build if `textutil` passes its include folder on.

## Assignment

- [ ] A library target named `textutil` built from `src/textutil.cpp`
- [ ] It shares its `include` folder and its need for C++20 with the targets that link it
- [ ] A program target named `wc` built from `app/wc.cpp`, linking `textutil`
- [ ] No `include_directories` or `CMAKE_CXX_STANDARD`: everything goes through targets
