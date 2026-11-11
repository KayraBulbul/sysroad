// Races the plain loops against yours over 16 MB of text. You won't need to
// change this file.
#include <cstdio>
#include <functional>
#include <string>

#include "bench.hpp"
#include "simd.hpp"

namespace original {
bool is_ascii(const char* s, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        if (static_cast<unsigned char>(s[i]) >= 128) {
            return false;
        }
    }
    return true;
}
std::size_t count_byte(const char* s, std::size_t n, char c) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < n; ++i) {
        count += s[i] == c;
    }
    return count;
}
}  // namespace original

static double best_ms(const std::function<void()>& f) {
    return measure(f, 7, 1e6).min / 1e6;
}

static void race(const char* what, double before, double after, bool same) {
    if (!same) {
        std::printf("%-26s %8.2f ms   (yours gives a different answer)\n", what, before);
        return;
    }
    // 16 MB in `after` milliseconds; bytes per nanosecond is GB/s.
    std::printf("%-26s %8.2f ms %8.2f ms   %.1fx  (%.1f GB/s)\n", what, before, after,
                before / after, (16u << 20) / (after * 1e6));
}

int main() {
    std::string text;
    text.reserve(16u << 20);
    while (text.size() < (16u << 20)) {
        text += "the quick brown fox jumps over the lazy dog\n";
    }
    const char* s = text.data();
    std::size_t n = text.size();

    std::printf("%-26s %11s %11s\n", "16 MB of text", "plain", "yours");
    race("is_ascii", best_ms([&] { do_not_optimize(original::is_ascii(s, n)); }),
         best_ms([&] { do_not_optimize(is_ascii(s, n)); }),
         is_ascii(s, n) == original::is_ascii(s, n));
    race("count_byte '\\n'", best_ms([&] { do_not_optimize(original::count_byte(s, n, '\n')); }),
         best_ms([&] { do_not_optimize(count_byte(s, n, '\n')); }),
         count_byte(s, n, '\n') == original::count_byte(s, n, '\n'));
}
