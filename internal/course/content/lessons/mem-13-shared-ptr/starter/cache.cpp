#include "cache.hpp"

std::shared_ptr<const std::string> Cache::get(const std::string& name) {
    // TODO: return the live entry if there is one; otherwise load, remember, return.
    (void)name;
    return nullptr;
}

std::size_t Cache::in_use() {
    // TODO
    return 0;
}
