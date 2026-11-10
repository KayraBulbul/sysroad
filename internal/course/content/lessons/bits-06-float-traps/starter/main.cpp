// Calls each function with an input that hits its bug. You won't need to change this file.
#include <format>
#include <iostream>
#include <limits>

#include "traps.hpp"

int main() {
    double nan = std::numeric_limits<double>::quiet_NaN();
    std::cout << std::format("0.1 + 0.2 is {}\n", 0.1 + 0.2);
    std::cout << std::format("close_enough(0.1 + 0.2, 0.3)     {}\n", close_enough(0.1 + 0.2, 0.3));
    std::cout << std::format("elapsed_seconds(36000)           {}\n", elapsed_seconds(36000));
    std::cout << std::format("largest({{nan, 1, 2}})             {}\n", largest({nan, 1, 2}));
    std::cout << std::format("to_cents(0.29)                   {}\n", to_cents(0.29));
    auto id = parse_id("9007199254740993");
    std::cout << std::format("parse_id(\"9007199254740993\")     {}\n", id ? *id : 0);
}
