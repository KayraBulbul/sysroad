cmake_minimum_required(VERSION 3.20)
project(textutil LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug, Release, RelWithDebInfo or MinSizeRel" FORCE)
endif()

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

option(SANITIZE "Build with AddressSanitizer and UBSan" OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# TODO: fetch doctest v2.4.11 with FetchContent.

add_library(textutil src/textutil.cpp)
target_include_directories(textutil PUBLIC include)
target_compile_features(textutil PUBLIC cxx_std_20)
target_link_libraries(textutil PRIVATE project_warnings)

add_executable(wc app/wc.cpp)
target_link_libraries(wc PRIVATE textutil project_warnings)

enable_testing()
add_subdirectory(tests)
