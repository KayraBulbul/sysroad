// Counts what growing a vector and moving a Line cost. You won't need to change this file.
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "alloc_count.hpp"
#include "records.hpp"

int main() {
    std::vector<Record> made;
    made.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        made.emplace_back("student number " + std::to_string(i) + " of this year",
                          std::vector<int>{70, 80, 90});
    }

    AllocScope s;
    std::vector<Record> records;  // no reserve: it grows as it goes
    for (Record& r : made) {
        records.push_back(std::move(r));
    }
    std::printf("growing a vector to 1000 Records: %ld allocations\n", s.news());
    std::printf("  (about 11 are the vector's own buffers; the rest are copies)\n");

    Line a("a log line long enough that its text lives on the heap");
    AllocScope s2;
    Line b = std::move(a);
    std::printf("moving a Line: %ld allocation%s\n", s2.news(), s2.news() == 1 ? "" : "s");
}
