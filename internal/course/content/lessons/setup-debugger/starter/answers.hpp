#pragma once
// Your answers. Replace each 0 or "" with what the debugger shows you.

#include <cstdint>
#include <string_view>

// 1. What does walk(1, 1000) return? (print/expr a call)
constexpr std::uint32_t kWalk1000 = 0;

// 2. In checksum(100), the value of acc when the line `acc = acc * 31 ...`
//    is reached with i == 37. (a conditional breakpoint)
constexpr std::uint64_t kAccAt37 = 0;

// 3. The line number in drill.cpp where the program crashes. (bt)
constexpr int kCrashLine = 0;

// 4. The name level_of was called with when it crashed. (frame, print)
constexpr std::string_view kMissingName = "";
