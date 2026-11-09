#pragma once
// Counts what new and delete do, by replacing the global operator new and
// operator delete (see alloc_count.cpp). You won't need to change this file.

#include <cstddef>

struct AllocStats {
    long news = 0;          // calls to new and new[]
    long deletes = 0;       // calls to delete and delete[] on a non-null pointer
    std::size_t bytes = 0;  // bytes asked for, in total
};

// Totals since the program started.
AllocStats alloc_stats();

// Counts from the moment it's made:  AllocScope s;  ...  s.news()
class AllocScope {
  public:
    AllocScope() : start_(alloc_stats()) {}
    long news() const {
        return alloc_stats().news - start_.news;
    }
    long deletes() const {
        return alloc_stats().deletes - start_.deletes;
    }
    // Allocations made since, and not freed yet.
    long live() const {
        return news() - deletes();
    }
    std::size_t bytes() const {
        return alloc_stats().bytes - start_.bytes;
    }

  private:
    AllocStats start_;
};
