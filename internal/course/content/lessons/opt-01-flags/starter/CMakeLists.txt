cmake_minimum_required(VERSION 3.20)
project(brighten LANGUAGES CXX)

# TODO: build as RelWithDebInfo when no build type is given.

# TODO: option(PROFILING ...), off by default. When it's on, compile
# everything with -fno-omit-frame-pointer.

# TODO: option(LTO ...), off by default. When it's on, and
# check_ipo_supported says LTO works here, turn on
# CMAKE_INTERPROCEDURAL_OPTIMIZATION.

add_library(pixel src/pixel.cpp)
target_include_directories(pixel PUBLIC include)
target_compile_features(pixel PUBLIC cxx_std_20)

add_executable(brighten app/main.cpp)
target_link_libraries(brighten PRIVATE pixel)
