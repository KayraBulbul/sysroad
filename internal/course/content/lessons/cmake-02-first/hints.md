The four commands from the lesson are the whole file. Change `tool` to
`hello` and list this project's two `.cpp` files.
---
If the build says `no member named 'starts_with'`, the compiler is in
C++17 mode, clang's default. `target_compile_features(hello PRIVATE cxx_std_20)`
asks for C++20, and it has to come after `add_executable`, since it
refers to the target by name.
---
```cmake
cmake_minimum_required(VERSION 3.20)
project(hello LANGUAGES CXX)

add_executable(hello main.cpp greet.cpp)
target_compile_features(hello PRIVATE cxx_std_20)
```
