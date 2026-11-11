#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstdint>
#include <vector>

// The shape of a cache. All three are powers of two, and size is a
// multiple of line * ways.
struct CacheConfig {
    std::uint64_t size;  // bytes of data it holds
    std::uint64_t line;  // bytes per line
    std::uint64_t ways;  // lines per set
};

// A simulated cache with least-recently-used (LRU) replacement. It
// doesn't store any data, only which lines it holds.
class Cache {
  public:
    explicit Cache(CacheConfig config);

    // How many sets there are: size / (line * ways).
    std::uint64_t sets() const;
    // Which set the line holding addr goes in.
    std::uint64_t set_of(std::uint64_t addr) const;
    // The tag that tells apart the lines that share that set.
    std::uint64_t tag_of(std::uint64_t addr) const;

    // Reads the byte at addr. Returns true on a hit. On a miss the line is
    // loaded, and if its set is full the least recently used line goes.
    bool access(std::uint64_t addr);

    std::uint64_t hits() const {
        return hits_;
    }
    std::uint64_t misses() const {
        return misses_;
    }

  private:
    struct Line {
        std::uint64_t tag = 0;
        std::uint64_t last_used = 0;  // the value of time_ when it was last accessed
    };

    CacheConfig config_;
    // One vector per set. Each holds the lines in that set, up to `ways` of them.
    std::vector<std::vector<Line>> sets_;
    std::uint64_t time_ = 0;  // goes up by one on every access
    std::uint64_t hits_ = 0;
    std::uint64_t misses_ = 0;
};
