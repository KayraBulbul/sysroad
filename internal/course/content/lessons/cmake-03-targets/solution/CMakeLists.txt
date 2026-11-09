cmake_minimum_required(VERSION 3.20)
project(textutil LANGUAGES CXX)

add_library(textutil src/textutil.cpp)
target_include_directories(textutil PUBLIC include)
target_compile_features(textutil PUBLIC cxx_std_20)

add_executable(wc app/wc.cpp)
target_link_libraries(wc PRIVATE textutil)
