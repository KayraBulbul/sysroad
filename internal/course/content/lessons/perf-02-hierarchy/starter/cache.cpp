#include "cache.hpp"

Cache::Cache(CacheConfig config) : config_(config) {
    // TODO: one empty vector per set.
}

std::uint64_t Cache::sets() const {
    // TODO
    (void)config_;
    return 1;
}

std::uint64_t Cache::set_of(std::uint64_t addr) const {
    // TODO
    (void)addr;
    return 0;
}

std::uint64_t Cache::tag_of(std::uint64_t addr) const {
    // TODO
    (void)addr;
    return 0;
}

bool Cache::access(std::uint64_t addr) {
    // TODO
    (void)addr;
    (void)time_;
    return false;
}
