cmake_minimum_required(VERSION 3.20)
project(hello LANGUAGES CXX)

add_executable(hello main.cpp greet.cpp)
target_compile_features(hello PRIVATE cxx_std_20)
