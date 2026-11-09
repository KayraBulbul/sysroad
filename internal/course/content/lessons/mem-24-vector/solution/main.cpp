// Fills some Vectors and counts allocations. You won't need to change this file.
#include <cstdio>
#include <memory>
#include <string>

#include "alloc_count.hpp"
#include "vector.hpp"

int main() {
    {
        AllocScope a;
        Vector<int> v;
        for (int i = 0; i < 1000; ++i) {
            v.push_back(i);
        }
        std::printf("1000 ints:    size %zu, capacity %zu, %ld allocations\n", v.size(),
                    v.capacity(), a.news());
    }
    {
        Vector<std::string> words;
        words.push_back("placement");
        words.emplace_back(3, 'n');  // std::string(3, 'n')
        words.push_back(words[0]);   // a reference into itself, while growing
        std::printf("strings:      ");
        for (std::size_t i = 0; i < words.size(); ++i) {
            std::printf("\"%s\" ", words[i].c_str());
        }
        std::printf("\n");
    }
    {
        Vector<std::unique_ptr<int>> owners;  // move-only elements
        for (int i = 0; i < 5; ++i) {
            owners.push_back(std::make_unique<int>(i * i));
        }
        std::printf("unique_ptrs:  the last one points at %d\n", *owners.back());
    }
}
