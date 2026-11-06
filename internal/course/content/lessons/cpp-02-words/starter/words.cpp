#include <cctype>
#include <cstddef>
#include <string_view>

// Counts words: runs of one or more non-whitespace bytes.
//   "hello world"  ->  2
//   "   "          ->  0
std::size_t count_words(std::string_view text) {
    std::size_t count = 0;
    // TODO
    return count;
}
