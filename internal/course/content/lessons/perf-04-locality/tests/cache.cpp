// The simulator from lesson 5.1.2.
#include "cache.hpp"

Cache::Cache(CacheConfig config) : config_(config), sets_(sets()) {}

std::uint64_t Cache::sets() const {
    return config_.size / (config_.line * config_.ways);
}

std::uint64_t Cache::set_of(std::uint64_t addr) const {
    return addr / config_.line % sets();
}

std::uint64_t Cache::tag_of(std::uint64_t addr) const {
    return addr / config_.line / sets();
}

bool Cache::access(std::uint64_t addr) {
    ++time_;
    std::vector<Line>& set = sets_[set_of(addr)];
    std::uint64_t tag = tag_of(addr);
    for (Line& l : set) {
        if (l.tag == tag) {
            l.last_used = time_;
            ++hits_;
            return true;
        }
    }
    ++misses_;
    if (set.size() < config_.ways) {
        set.push_back({tag, time_});
        return false;
    }
    Line* oldest = &set[0];
    for (Line& l : set) {
        if (l.last_used < oldest->last_used) {
            oldest = &l;
        }
    }
    *oldest = {tag, time_};
    return false;
}
