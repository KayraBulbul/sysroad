#pragma once
// What opens the vault. Replace each "" or 0 with what the debugger shows you.

#include <string_view>

// Phase 1: the word.
constexpr std::string_view kWord = "nightjar";

// Phase 2: the number.
constexpr long kNumber = 3758085031;

// Phase 3: the index, 0 to 7.
constexpr int kIndex = 4;
