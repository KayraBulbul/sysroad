#include "cache.hpp"

std::shared_ptr<const std::string> Cache::get(const std::string& name) {
    if (auto it = entries_.find(name); it != entries_.end()) {
        if (auto alive = it->second.lock()) {
            return alive;
        }
    }
    auto fresh = std::make_shared<const std::string>(load_(name));
    ++loads_;
    entries_[name] = fresh;
    return fresh;
}

std::size_t Cache::in_use() {
    std::erase_if(entries_, [](const auto& kv) { return kv.second.expired(); });
    return entries_.size();
}
