cmake_minimum_required(VERSION 3.20)
project(textutil LANGUAGES CXX)

# One program, built from every source, and every target sees every header.
# It works, but nothing else can reuse the text code. Split it into a
# library and a program (see the lesson).
include_directories(include)
set(CMAKE_CXX_STANDARD 20)
add_executable(wc app/wc.cpp src/textutil.cpp)
