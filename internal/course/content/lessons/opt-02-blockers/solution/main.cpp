// Races the original versions against yours. You won't need to change this
// file.
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "bench.hpp"
#include "blockers.hpp"

// The originals, as they were in blockers.cpp.
namespace original {
void lowercase(char* s) {
    for (std::size_t i = 0; i < std::strlen(s); ++i) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = static_cast<char>(s[i] - 'A' + 'a');
        }
    }
}
void invert(Image& img) {
    for (std::size_t i = 0; i < img.size; ++i) {
        img.pixels[i] = static_cast<std::uint8_t>(255 - img.pixels[i]);
    }
}
void threshold(std::vector<std::uint8_t>& pixels, std::uint8_t level) {
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = pixels[i] < level ? 0 : 255;
    }
}
}  // namespace original

static double best_ms(const std::function<void()>& f) {
    return measure(f, 5, 1e6).min / 1e6;
}

static void race(const char* what, double before, double after) {
    std::printf("%-30s %9.3f ms %9.3f ms   %.1fx\n", what, before, after, before / after);
}

int main() {
    std::printf("%-30s %12s %12s\n", "", "original", "yours");

    std::string text(100'000, 'A');
    race("lowercase 100 KB", best_ms([&] {
             std::string s = text;
             original::lowercase(s.data());
             do_not_optimize(s.data());
         }),
         best_ms([&] {
             std::string s = text;
             lowercase(s.data());
             do_not_optimize(s.data());
         }));

    std::vector<std::uint8_t> px(4u << 20);
    for (std::size_t i = 0; i < px.size(); ++i) {
        px[i] = static_cast<std::uint8_t>(i * 7);
    }
    Image img{px.data(), px.size()};
    race("invert 4 MB", best_ms([&] { original::invert(img); }), best_ms([&] { invert(img); }));
    race("threshold 4 MB", best_ms([&] { original::threshold(px, 128); }),
         best_ms([&] { threshold(px, 128); }));
}
