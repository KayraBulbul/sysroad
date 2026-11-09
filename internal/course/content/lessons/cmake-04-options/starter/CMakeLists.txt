cmake_minimum_required(VERSION 3.20)
project(textutil LANGUAGES CXX)

# TODO: build as Debug when no build type is given.

# TODO: an INTERFACE target, project_warnings, with -Wall -Wextra -Wpedantic.

# TODO: option(SANITIZE ...), off by default. When it's on, compile and link
# everything with -fsanitize=address,undefined.

add_library(textutil src/textutil.cpp)
target_include_directories(textutil PUBLIC include)
target_compile_features(textutil PUBLIC cxx_std_20)

add_executable(wc app/wc.cpp)
target_link_libraries(wc PRIVATE textutil)
