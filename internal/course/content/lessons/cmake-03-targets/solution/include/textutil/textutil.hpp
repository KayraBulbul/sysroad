#pragma once
// The textutil library's interface: anything that links textutil includes
// this as "textutil/textutil.hpp". You won't need to change this file.

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace textutil {

struct Counts {
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t bytes = 0;

    bool operator==(const Counts&) const = default;
};

// Lines, words and bytes, the way wc counts them: a line is a '\n', and a
// word is a run of characters that aren't spaces.
Counts count(std::string_view text);

// The sum of several counts.
Counts total(std::span<const Counts> parts);

// The words of text, in order.
std::vector<std::string_view> split_words(std::string_view text);

// text without spaces at either end.
std::string_view trim(std::string_view text);

}  // namespace textutil
