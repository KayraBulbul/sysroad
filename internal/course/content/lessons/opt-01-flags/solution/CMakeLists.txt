cmake_minimum_required(VERSION 3.20)
project(brighten LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Debug, Release, RelWithDebInfo or MinSizeRel" FORCE)
endif()

option(PROFILING "Keep frame pointers, for perf and Instruments" OFF)
if(PROFILING)
  add_compile_options(-fno-omit-frame-pointer)
endif()

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

add_library(pixel src/pixel.cpp)
target_include_directories(pixel PUBLIC include)
target_compile_features(pixel PUBLIC cxx_std_20)

add_executable(brighten app/main.cpp)
target_link_libraries(brighten PRIVATE pixel)
