#pragma once
// A plain std::map count to time count_words against. Both are built the
// same way, so comparing them works on a fast PC and a slow laptop alike.

#include <chrono>
#include <map>

#include "wordfreq.hpp"

inline std::vector<WordCount> reference_count(const std::vector<std::string>& words) {
    std::map<std::string, long> seen;
    for (const std::string& w : words) {
        ++seen[w];
    }
    std::vector<WordCount> out;
    for (const auto& [word, count] : seen) {
        out.push_back({word, count});
    }
    return out;
}

// Seconds f takes to run once.
template <typename F> double seconds(F f) {
    auto start = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}
